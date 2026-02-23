// Fill out your copyright notice in the Description page of Project Settings.

#include "LlamaCppSubsystem.h"

//THIRD_PARTY_INCLUDES_START
#include "llama.h"
#include "common.h"
#include "ggml.h"
//THIRD_PARTY_INCLUDES_END
#include "sampling.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogLlamaRunner);

namespace
{
	FString ResolvePathPlaceholders(const FString& InPath)
	{
		if (InPath.IsEmpty())
		{
			return InPath;
		}

		FString ResolvedPath = InPath;

		const FString AbsProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		const FString AbsEngineDir = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());
		const FString AbsLaunchDir = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir());
		const FString AbsContentDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
		const FString AbsSavedDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());

		ResolvedPath = ResolvedPath.Replace(TEXT("{ProjectDir}"), *AbsProjectDir);
		ResolvedPath = ResolvedPath.Replace(TEXT("{EngineDir}"), *AbsEngineDir);
		ResolvedPath = ResolvedPath.Replace(TEXT("{LaunchDir}"), *AbsLaunchDir);
		ResolvedPath = ResolvedPath.Replace(TEXT("{ProjectContentDir}"), *AbsContentDir);
		ResolvedPath = ResolvedPath.Replace(TEXT("{ProjectSavedDir}"), *AbsSavedDir);

		FPaths::NormalizeFilename(ResolvedPath);
		FPaths::CollapseRelativeDirectories(ResolvedPath);

		if (FPaths::IsRelative(ResolvedPath))
		{
			ResolvedPath = FPaths::ConvertRelativePathToFull(AbsProjectDir, ResolvedPath);
		}

		return ResolvedPath;
	}
}

FLlamaModelState::FLlamaModelState()
	: N_Ctx(0)
{
	CommonMessages = new std::vector<common_chat_msg>();
}

FLlamaModelState::~FLlamaModelState()
{
	Shutdown();
	delete CommonMessages;
}

void FLlamaModelState::InitializeCommonParams()
{
	CommonParams = new common_params();

	// Check for command-line provided model path (if running the game from terminal)
	FString ModelPath;
	FString CommandLineModelPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("ModelPath="), CommandLineModelPath))
	{
		if (FPaths::IsRelative(CommandLineModelPath))
		{
			ModelPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), CommandLineModelPath);

			if (!FPaths::FileExists(ModelPath))
			{
				ModelPath = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), CommandLineModelPath);
			}
		}
		else
		{
			ModelPath = CommandLineModelPath;
		}

		UE_LOG(LogLlamaRunner, Display, TEXT("Using model path from command-line: %s"), *ModelPath);
	}
	else
	{
		ModelPath = ResolvePathPlaceholders(GeneralSettings->ModelPath.FilePath);
		UE_LOG(LogLlamaRunner, Display, TEXT("Using model path from project settings: %s"), *ModelPath);
	}

	CommonParams->model.path = TCHAR_TO_UTF8(*ModelPath);

	// Check for command-line override of context size
	int32 ContextSize;
	FString CommandLineContextSize;
	if (FParse::Value(FCommandLine::Get(), TEXT("ContextSize="), CommandLineContextSize))
	{
		ContextSize = FCString::Atoi(*CommandLineContextSize); //--> UE C String utilities
		UE_LOG(LogLlamaRunner, Display, TEXT("Using context size from command-line: %d"), ContextSize);
	}
	else
	{
		ContextSize = GeneralSettings->ContextSize;
		UE_LOG(LogLlamaRunner, Display, TEXT("Using context size from project settings: %d"), ContextSize);
	}

	CommonParams->n_ctx = ContextSize;
	CommonParams->n_batch = GeneralSettings->Batches;
	CommonParams->n_ubatch = FMath::Min(1024, GeneralSettings->Batches / 4);
	CommonParams->n_gpu_layers = -1; //GeneralSettings->GraphicalLayers; used with Llama.cpp

	CommonParams->warmup = true;
	//CommonParams->conversation = true; TODO ??

	CommonParams->n_predict = -1; // unlimited token generation

	const int32 NumLogicalCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
	const int32 OptimalThreadCount = FMath::Max(1, NumLogicalCores - GeneralSettings->ReservedCores);

	CommonParams->cpuparams.n_threads = OptimalThreadCount;
	CommonParams->cpuparams.priority = GGML_SCHED_PRIO_REALTIME;
	CommonParams->cpuparams_batch.n_threads = OptimalThreadCount;
	CommonParams->cpuparams_batch.priority = GGML_SCHED_PRIO_REALTIME;
	CommonParams->cpuparams.poll = 0;
	CommonParams->cpuparams_batch.poll = 0;

	CommonParams->no_perf = GeneralSettings->bIsNoPerf;
}

common_params_sampling FLlamaModelState::InitializeSamplerParams() const
{
	common_params_sampling* Params = new common_params_sampling();

	Params->seed = CurrentSamplerSeed;

	Params->temp = GeneralSettings->Temperature;
	Params->top_p = GeneralSettings->DecodingMethod == Traditional ? GeneralSettings->TopP : 1.0f;
	Params->min_p = GeneralSettings->DecodingMethod == MinP ? GeneralSettings->MinPValue : 0.0f;
	Params->top_k = GeneralSettings->bUseTopKFiltering && GeneralSettings->DecodingMethod == Traditional
		? GeneralSettings->TopK
		: 0;

	// Repetition penalties
	if (GeneralSettings->bUseRepetitionPenalty)
	{
		Params->penalty_last_n = GeneralSettings->PenaltyLastN;
		Params->penalty_repeat = GeneralSettings->PenaltyRepeat;
		Params->penalty_freq = GeneralSettings->PenaltyFrequency;
		Params->penalty_present = GeneralSettings->PenaltyPresent;
	}
	else
	{
		Params->penalty_last_n = 0;
		Params->penalty_repeat = 1.0f;
		Params->penalty_freq = 0.0f;
		Params->penalty_present = 0.0f;
	}

	// Mirostat
	if (GeneralSettings->SamplerMethod == Mirostat)
	{
		Params->mirostat = 2;
		Params->mirostat_tau = GeneralSettings->Tau;
		Params->mirostat_eta = GeneralSettings->Eta;
	}
	else
	{
		Params->mirostat = 0;
	}

	// Check for command-line provided grammar path (if running the game from terminal)

	FString GrammarPath;
	FString CommandLineGrammarPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("GrammarPath="), CommandLineGrammarPath))
	{
		if (FPaths::IsRelative(CommandLineGrammarPath))
		{
			GrammarPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), CommandLineGrammarPath);

			if (!FPaths::FileExists(GrammarPath))
			{
				GrammarPath = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), CommandLineGrammarPath);
			}
		}
		else
		{
			GrammarPath = CommandLineGrammarPath;
		}

		UE_LOG(LogLlamaRunner, Display, TEXT("Using grammar path from command-line: %s"), *GrammarPath);
	}
	else
	{
		GrammarPath = ResolvePathPlaceholders(GeneralSettings->GrammarPath.FilePath);
		if (!GrammarPath.IsEmpty())
		{
			UE_LOG(LogLlamaRunner, Display, TEXT("Using grammar path from project settings: %s"), *GrammarPath);
		}
	}

	if (FString GrammarContent; !GrammarPath.IsEmpty() &&
		FFileHelper::LoadFileToString(GrammarContent, *GrammarPath))
	{
		// Convert FString to std::string for grammar
		const std::string GrammarStr = TCHAR_TO_UTF8(*GrammarContent);
		Params->grammar = GrammarStr;
		UE_LOG(LogLlamaRunner, Display, TEXT("Loaded grammar from: %s"), *GrammarPath);
	}
	else if (!GrammarPath.IsEmpty())
	{
		UE_LOG(LogLlamaRunner, Warning, TEXT("Failed to load grammar file: %s"),
			*GrammarPath);
	}

	return *Params;
}

//TODO incoming issues with Thread pooling for CPU while using Llama on CUDA 
void FLlamaModelState::InitializeThreadPools()
{
	ggml_threadpool_params tpp = ggml_threadpool_params_from_cpu_params(CommonParams->cpuparams);
	ggml_threadpool_params tpp_batch = ggml_threadpool_params_from_cpu_params(CommonParams->cpuparams_batch);

	set_process_priority(CommonParams->cpuparams.priority);

	ThreadPoolBatch = nullptr;
	if (!ggml_threadpool_params_match(&tpp, &tpp_batch))
	{
		ThreadPoolBatch = ggml_threadpool_new(&tpp_batch);
		if (!ThreadPoolBatch)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Failed to create batch threadpool"));
			return;
		}
		tpp.paused = true;
	}

	ThreadPool = ggml_threadpool_new(&tpp);
	if (!ThreadPool)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to create main threadpool"));
		if (ThreadPoolBatch)
		{
			ggml_threadpool_free(ThreadPoolBatch);
			ThreadPoolBatch = nullptr;
		}
		return;
	}

	llama_attach_threadpool(Context.get(), ThreadPool, ThreadPoolBatch);

	UE_LOG(LogLlamaRunner, Display, TEXT("Thread pools initialized and attached"));
}

bool FLlamaModelState::Initialize()
{
	InitializeCommonParams();

	// Load model and context
	common_init_result InitResult = common_init_from_params(*CommonParams);
	Model = std::move(InitResult.model);
	Context = std::move(InitResult.context);

	if (!Model || !Context)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to load model or create context"));
		return false;
	}

	// Load sampler and grammar with user settings
	CommonSampler = common_sampler_init(Model.get(), InitializeSamplerParams());

	if (!CommonSampler)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to initialize sampler"));
		return false;
	}

	N_Ctx = llama_n_ctx(Context.get());

	InitializeThreadPools();

	UE_LOG(LogLlamaRunner, Display, TEXT("Model state initialized successfully"));
	return true;
}

void FLlamaModelState::Shutdown()
{
	if (CommonSampler)
	{
		common_sampler_free(CommonSampler);
		CommonSampler = nullptr;
	}

	if (Context)
	{
		Context.reset(); // calls llama_free()
		Context = nullptr;
	}

	if (Model)
	{
		Model.reset();
		Model = nullptr;
	}

	if (CommonParams)
	{
		delete CommonParams;
		CommonParams = nullptr;
	}

	if (ThreadPool)
	{
		ggml_threadpool_free(ThreadPool);
		ThreadPool = nullptr;
	}

	if (ThreadPoolBatch)
	{
		ggml_threadpool_free(ThreadPoolBatch);
		ThreadPoolBatch = nullptr;
	}

	SessionTokens.clear();
	SessionTokensConsumed = 0;
}

//TODO: should we use string_process_escapes at some point?
//TODO: miss SessionTokens impl.
FString FLlamaModelState::Generate(const FContextWarningCallback& WarningCallback) const
{
	UE_LOG(LogLlamaRunner, Verbose, TEXT("Generate(): Begin"));

	FString Response;

	const int32 CurrentTokensUsed = GetContextTokensUsed();
	const float CurrentUsagePercent = GetContextUsagePercent();

	UE_LOG(LogLlamaRunner, Display,
		TEXT("Context usage before generation: %d/%d tokens (%.1f%%)"),
		CurrentTokensUsed, N_Ctx, CurrentUsagePercent);

	// Tokenize the conversation
	std::vector<llama_token> EmbeddedInput;
	{
		if (!CommonMessages->empty())
		{
			common_chat_templates_ptr ChatTemplates = common_chat_templates_init(
				Model.get(),
				"",
				"",
				""
				);

			if (!ChatTemplates)
			{
				UE_LOG(LogLlamaRunner, Error, TEXT("Failed to initialize chat templates"));
				return Response;
			}

			common_chat_templates_inputs TemplateInputs;
			TemplateInputs.messages = *CommonMessages;
			TemplateInputs.add_generation_prompt = true;
			TemplateInputs.use_jinja = false;
			TemplateInputs.add_bos = true;
			TemplateInputs.add_eos = false;

			common_chat_params ChatParams = common_chat_templates_apply(
				ChatTemplates.get(),
				TemplateInputs
				);

			EmbeddedInput = common_tokenize(
				Context.get(),
				ChatParams.prompt,
				true,
				true
				);

			UE_LOG(LogLlamaRunner, Display, TEXT("Tokenized %d messages into %d tokens"),
				static_cast<int>(CommonMessages->size()), static_cast<int>(EmbeddedInput.size()));

			// Check if prompt + minimum generation buffer can fit
			// 256 fits the current grammar max length and 512 would be leaving ~55% more 
			// if we know what to increase the grammar length (i.e. reasoning or verbal-interaction)
			if (constexpr int32 MinGenerationBuffer = 256;
				!CanFitTokens(static_cast<int32>(EmbeddedInput.size()), MinGenerationBuffer))
			{
				const int32 TokensNeeded = static_cast<int32>(EmbeddedInput.size()) + MinGenerationBuffer;
				const int32 TokensAvailable = GetContextTokensRemaining();

				UE_LOG(LogLlamaRunner, Error,
					TEXT("Context overflow: Need %d tokens (%d prompt + %d buffer), but only %d available"),
					TokensNeeded, static_cast<int32>(EmbeddedInput.size()), MinGenerationBuffer, TokensAvailable);

				return Response; // empty response
			}

			// Check if usage will exceed 80% after processing this prompt
			const int32 TokensAfterPrompt = CurrentTokensUsed + static_cast<int32>(EmbeddedInput.size());
			const float UsageAfterPrompt = (static_cast<float>(TokensAfterPrompt) / static_cast<float>(N_Ctx)) * 100.0f;

			if (UsageAfterPrompt >= 80.0f)
			{
				UE_LOG(LogLlamaRunner, Warning,
					TEXT("Context usage warning: %.1f%% after adding prompt (%d/%d tokens)"),
					UsageAfterPrompt, TokensAfterPrompt, N_Ctx);

				// Broadcast warning via callback if provided
				if (WarningCallback)
				{
					WarningCallback(TokensAfterPrompt, N_Ctx, UsageAfterPrompt);
				}
			}
		}
		else
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("No conversation messages available"));
			return Response;
		}
	}

	std::vector<llama_token> Embedded;
	int32 N_Past = 0;                         // Number of tokens processed so far
	int32 N_Remain = CommonParams->n_predict; // Remaining tokens to generate
	int32 N_Consumed = 0;                     // Tokens consumed from input

	std::ostringstream Assistant_SS;

	// gen loop
	while (N_Remain != 0)
	{
		if (!Embedded.empty())
		{
			if (N_Past + static_cast<int>(Embedded.size()) >= N_Ctx)
			{
				UE_LOG(LogLlamaRunner, Warning, TEXT("Context limit reached, stopping generation"));
				break;
			}

			// batch processing
			for (int i = 0; i < static_cast<int>(Embedded.size()); i += CommonParams->n_batch)
			{
				int N_Eval = static_cast<int>(Embedded.size()) - i;
				if (N_Eval > CommonParams->n_batch)
				{
					N_Eval = CommonParams->n_batch;
				}

				if (llama_decode(Context.get(), llama_batch_get_one(&Embedded[i], N_Eval)))
				{
					UE_LOG(LogLlamaRunner, Error, TEXT("Failed to decode"));
					return Response;
				}

				N_Past += N_Eval;
			}

			Embedded.clear();
		}

		// generate next token or process input
		if (static_cast<int>(EmbeddedInput.size()) <= N_Consumed)
		{
			const llama_token id = common_sampler_sample(CommonSampler, Context.get(), -1);
			common_sampler_accept(CommonSampler, id, true);

			Embedded.push_back(id);

			--N_Remain;

			if (llama_vocab_is_eog(llama_model_get_vocab(Model.get()), id))
			{
				UE_LOG(LogLlamaRunner, Display, TEXT("End of generation token found"));
				break;
			}

			Assistant_SS << common_token_to_piece(Context.get(), id, false);
		}
		else
		{
			while (static_cast<int>(EmbeddedInput.size()) > N_Consumed)
			{
				Embedded.push_back(EmbeddedInput[N_Consumed]);
				common_sampler_accept(CommonSampler, EmbeddedInput[N_Consumed], false);

				++N_Consumed;

				if (static_cast<int>(Embedded.size()) >= CommonParams->n_batch)
				{
					break;
				}
			}
		}
	}

	// Convert response to FString
	if (!Assistant_SS.str().empty())
	{
		Response = FString(Assistant_SS.str().c_str());
		UE_LOG(LogLlamaRunner, Display, TEXT("Generated response: %d characters"), Response.Len());
	}

	return Response;
}

void FLlamaModelState::ClearCache()
{
	if (Context)
	{
		llama_memory_clear(llama_get_memory(Context.get()), true);
		SessionTokens.clear();
		SessionTokensConsumed = 0;
		UE_LOG(LogLlamaRunner, Display, TEXT("KV cache cleared"));
	}
}

void FLlamaModelState::ResetSampler() const
{
	if (CommonSampler)
	{
		common_sampler_reset(CommonSampler);
		UE_LOG(LogLlamaRunner, Display, TEXT("Sampler reset"));
	}
}

void FLlamaModelState::ReinitializeSamplerWithNewSeed()
{
	CurrentSamplerSeed = GenerateNewSeed();

	if (CommonSampler)
	{
		common_sampler_free(CommonSampler);
		CommonSampler = nullptr;
	}

	CommonSampler = common_sampler_init(Model.get(), InitializeSamplerParams());

	if (CommonSampler)
	{
		UE_LOG(LogLlamaRunner, Display, TEXT("Sampler reinitialized with new seed: %u"), CurrentSamplerSeed);
	}
	else
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to reinitialize sampler with new seed"));
	}
}

int32 FLlamaModelState::GetContextTokensUsed() const
{
	if (!Context)
	{
		return 0;
	}

	// Get the position range in the KV cache for default sequence (id 0)
	const llama_memory_t Memory = llama_get_memory(Context.get());
	const llama_pos MaxPos = llama_memory_seq_pos_max(Memory, 0);

	if (MaxPos < 0)
	{
		return 0;
	}

	// https://github.com/ggml-org/llama.cpp/pull/13194
	// For Sliding Window Attention (SWA) caches, positions may not start at 0
	// Get the minimum position to calculate accurate token count
	const llama_pos MinPos = llama_memory_seq_pos_min(Memory, 0);
	if (MinPos < 0)
	{
		return 0; // Defensive: shouldn't happen if MaxPos >= 0
	}

	return MaxPos - MinPos + 1;
}

int32 FLlamaModelState::GetContextTokensRemaining() const
{
	return N_Ctx - GetContextTokensUsed();
}

float FLlamaModelState::GetContextUsagePercent() const
{
	if (N_Ctx <= 0)
	{
		return 0.0f;
	}
	return (static_cast<float>(GetContextTokensUsed()) / static_cast<float>(N_Ctx)) * 100.0f;
}

bool FLlamaModelState::CanFitTokens(const int32 TokenCount, const int32 MinGenerationBuffer) const
{
	const int32 Remaining = GetContextTokensRemaining();
	return Remaining >= (TokenCount + MinGenerationBuffer);
}

void FLlamaModelState::AddChatAndFormat(const EChatRole Role, const FString& Content) const
{
	const std::string RoleStr = TCHAR_TO_UTF8(*UEnum::GetValueAsString(Role).ToLower());
	const std::string ContentStr = TCHAR_TO_UTF8(*Content);

	common_chat_templates_ptr ChatTemplates = common_chat_templates_init(
		Model.get(),
		"",
		"",
		""
		);

	const common_chat_msg NewMessage{ RoleStr, ContentStr };
	auto Formatted = common_chat_format_single(
		ChatTemplates.get(),
		*CommonMessages,
		NewMessage,
		RoleStr == "user",
		false); // new API
	CommonMessages->push_back({ RoleStr, ContentStr });

	UE_LOG(LogLlamaRunner, Verbose, TEXT("Added message - Role: %s, Length: %d"),
		*FString(RoleStr.c_str()), Content.Len());
}

// -------------- FLlamaInferenceThread --------------

FLlamaInferenceThread::FLlamaInferenceThread(ULlamaCppSubsystem* InOwner)
	: Owner(InOwner)
	, bShutdown(false)
	, bIsProcessing(false)
	, NextRequestId(1)
{
	WakeUpEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);
	Thread = FRunnableThread::Create(this, TEXT("LlamaInferenceThread"),
		0, TPri_AboveNormal);
}

FLlamaInferenceThread::~FLlamaInferenceThread()
{
	FLlamaInferenceThread::Stop();

	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	if (WakeUpEvent)
	{
		FGenericPlatformProcess::ReturnSynchEventToPool(WakeUpEvent);
		WakeUpEvent = nullptr;
	}
}

// --- LOGS
void FLlamaInferenceThread::BindLlamaRunnerLogs()
{
	llama_log_set([](const ggml_log_level LlamaLogLevel, const char* LlamaLogText, void*) {
		if (LlamaLogLevel >= GGML_LOG_LEVEL_ERROR)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("%hs"), LlamaLogText);
		}
		else if (LlamaLogLevel >= GGML_LOG_LEVEL_WARN)
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("%hs"), LlamaLogText);
		}
		else if (LlamaLogLevel >= GGML_LOG_LEVEL_INFO)
		{
			UE_LOG(LogLlamaRunner, Display, TEXT("%hs"), LlamaLogText);
		}
	}, nullptr);
}

void FLlamaInferenceThread::LogSamplingAndGenerationPerformances() const
{
	UE_LOG(LogLlamaRunner, Display, TEXT("--- Sampler Performance ---"));
	common_perf_print(ModelState->Context.get(), ModelState->CommonSampler);
}

// --- END LOGS

bool FLlamaInferenceThread::Init()
{
	BindLlamaRunnerLogs();

	ModelState = MakeUnique<FLlamaModelState>();

	if (!ModelState->Initialize())
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to initialize model state"));
		return false;
	}

	UE_LOG(LogLlamaRunner, Display, TEXT("Llama inference thread initialized"));
	return true;
}

uint32 FLlamaInferenceThread::Run()
{
	while (!bShutdown)
	{
		if (FLlamaCommand Command; CommandQueue.Dequeue(Command))
		{
			bIsProcessing = true;

			switch (Command.Type)
			{
				case ELlamaCommandType::ContinueChat:
					ProcessContinueChat(Command);
					#if !UE_BUILD_SHIPPING
					if (!GeneralSettings->bIsNoPerf)
					{
						LogSamplingAndGenerationPerformances();
					}
					#endif
					break;
				case ELlamaCommandType::SwitchCharacter:
					ProcessSwitchCharacter(Command);
					break;

				case ELlamaCommandType::ClearHistory:
					ProcessClearHistory(Command);
					break;

				case ELlamaCommandType::Shutdown:
					bShutdown = true;
					break;
			}

			bIsProcessing = false;
		}
		else
		{
			// Sleep when idle, wake every 50ms to check for work
			WakeUpEvent->Wait(50);
		}
	}

	return 0;
}

void FLlamaInferenceThread::Stop()
{
	bShutdown = true;

	if (WakeUpEvent)
	{
		WakeUpEvent->Trigger();
	}
}

void FLlamaInferenceThread::Exit()
{
	UE_LOG(LogLlamaRunner, Display, TEXT("Llama inference thread exiting"));
}

void FLlamaInferenceThread::ProcessContinueChat(const FLlamaCommand& Command)
{
	if (!ModelState || !ModelState->IsValid())
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Model state invalid"));
		return;
	}

	if (!bHasActiveConversation)
	{
		UE_LOG(LogLlamaRunner, Warning,
			TEXT("No active conversation, initializing with default character"));

		if (!CurrentSystemPrompt.IsEmpty())
		{
			ModelState->AddChatAndFormat(System, CurrentSystemPrompt);

			for (const auto& Example : CurrentFewShots)
			{
				ModelState->AddChatAndFormat(Example.ChatRole, Example.Content);
			}
		}

		bHasActiveConversation = true;
	}

	ModelState->AddChatAndFormat(User, Command.UserPrompt);

	auto WarningCallback = [Owner = Owner](int32 TokensUsed, int32 TokensTotal, float UsagePercent) {
		AsyncTask(ENamedThreads::GameThread,
			[Owner, TokensUsed, TokensTotal, UsagePercent] {
				if (Owner)
				{
					Owner->OnContextWarning.Broadcast(TokensUsed, TokensTotal, UsagePercent);
				}
			});
	};

	const double StartTime = FPlatformTime::Seconds();
	FString Response = ModelState->Generate(WarningCallback);
	const double EndTime = FPlatformTime::Seconds();
	const double InferenceTimeSeconds = EndTime - StartTime;

	if (!Response.IsEmpty())
	{
		ModelState->AddChatAndFormat(Assistant, Response);

		// Write timing to file
		const FString TimingLine = FString::Printf(TEXT("%.2f\n"), InferenceTimeSeconds);
		const FString TimingsFilePath = FPaths::ProjectDir() / TEXT("timings.txt");
		FFileHelper::SaveStringToFile(TimingLine, *TimingsFilePath, FFileHelper::EEncodingOptions::AutoDetect,
			&IFileManager::Get(), EFileWrite::FILEWRITE_Append);

		AsyncTask(ENamedThreads::GameThread,
			[Owner = Owner, Response, RequestId = Command.RequestId] {
				if (Owner)
				{
					Owner->OnInferenceComplete.Broadcast(Response);
					Owner->OnInferenceTimingCaptured.Broadcast();
					Owner->LastRequestId = RequestId;
				}
			});
	}
	else
	{
		// Empty response indicates context overflow or generation failure
		const int32 TokensUsed = ModelState->GetContextTokensUsed();
		const int32 TokensTotal = ModelState->GetContextTokensTotal();
		const int32 TokensRemaining = ModelState->GetContextTokensRemaining();

		FString ErrorMsg = FString::Printf(
			TEXT("Generation failed - likely context overflow. Tokens: %d/%d (remaining: %d)"),
			TokensUsed, TokensTotal, TokensRemaining);

		UE_LOG(LogLlamaRunner, Error, TEXT("%s"), *ErrorMsg);

		// Always broadcast error when generation fails
		AsyncTask(ENamedThreads::GameThread,
			[Owner = Owner, TokensUsed, TokensTotal, TokensRemaining] {
				if (Owner)
				{
					Owner->OnContextError.Broadcast(TokensUsed, TokensTotal, TokensRemaining);
				}
			});
	}
}

void FLlamaInferenceThread::ProcessSwitchCharacter(const FLlamaCommand& Command)
{
	if (!ModelState || !ModelState->IsValid())
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Model state invalid"));
		return;
	}

	UE_LOG(LogLlamaRunner, Display,
		TEXT("Switching character: clearing conversation and KV cache"));

	// Clear previous conversation
	ModelState->ClearMessages();
	ModelState->ClearCache();
	ModelState->ResetSampler();

	// Update current character context
	CurrentSystemPrompt = Command.SystemPrompt;
	CurrentFewShots = Command.FewShotExamples;

	// Initialize new conversation
	if (!CurrentSystemPrompt.IsEmpty())
	{
		ModelState->AddChatAndFormat(System, CurrentSystemPrompt);
	}

	// Add few-shot examples
	for (const auto& Example : CurrentFewShots)
	{
		ModelState->AddChatAndFormat(Example.ChatRole, Example.Content);
	}

	bHasActiveConversation = true;

	// Process initial user prompt if provided
	if (!Command.UserPrompt.IsEmpty())
	{
		ProcessContinueChat(Command);
		#if !UE_BUILD_SHIPPING
		if (!GeneralSettings->bIsNoPerf)
		{
			LogSamplingAndGenerationPerformances();
		}
		#endif
	}
	else
	{
		// notify character switch is complete
		AsyncTask(ENamedThreads::GameThread,
			[Owner = Owner, RequestId = Command.RequestId] {
				if (Owner)
				{
					Owner->LastRequestId = RequestId;
					UE_LOG(LogLlamaRunner, Display,
						TEXT("Character switched successfully"));
				}
			});
	}
}

void FLlamaInferenceThread::ProcessClearHistory(const FLlamaCommand& Command)
{
	if (!ModelState || !ModelState->IsValid())
	{
		return;
	}

	ModelState->ClearMessages();
	ModelState->ClearCache();

	if (Command.bReseedOnClear)
	{
		ModelState->ReinitializeSamplerWithNewSeed();
	}
	else
	{
		ModelState->ResetSampler();
	}

	bHasActiveConversation = false;

	UE_LOG(LogLlamaRunner, Display, TEXT("Chat history cleared (reseed: %s)"),
		Command.bReseedOnClear ? TEXT("true") : TEXT("false"));
}

uint32 FLlamaInferenceThread::QueueCommand(const FLlamaCommand& Command)
{
	const uint32 RequestId = NextRequestId.fetch_add(1);

	FLlamaCommand CommandWithId = Command;
	CommandWithId.RequestId = RequestId;

	CommandQueue.Enqueue(MoveTemp(CommandWithId));

	if (WakeUpEvent)
	{
		WakeUpEvent->Trigger();
	}

	return RequestId;
}

// -------------- ULlamaCppSubsystem --------------

bool ULlamaCppSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void ULlamaCppSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GeneralSettings->ModelPath.FilePath.IsEmpty())
	{
		UE_LOG(LogLlamaRunner, Warning,
			TEXT("Model path empty, skipping initialization"));
		return;
	}

	// will initialize the model
	InferenceThread = MakeUnique<FLlamaInferenceThread>(this);

	UE_LOG(LogLlamaRunner, Display, TEXT("LlamaCppSubsystem initialized"));
}

void ULlamaCppSubsystem::Deinitialize()
{
	if (InferenceThread)
	{
		FLlamaCommand ShutdownCommand;
		ShutdownCommand.Type = ELlamaCommandType::Shutdown;
		InferenceThread->QueueCommand(ShutdownCommand);
	}

	InferenceThread.Reset();

	Super::Deinitialize();
}

void ULlamaCppSubsystem::ContinueConversation(const FString& UserPrompt) const
{
	if (!InferenceThread)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Inference thread not initialized"));
		return;
	}

	FLlamaCommand Command;
	Command.Type = ELlamaCommandType::ContinueChat;
	Command.UserPrompt = UserPrompt;

	const uint32 RequestId = InferenceThread->QueueCommand(Command);
	UE_LOG(LogLlamaRunner, Display,
		TEXT("Queued continue conversation request %u"), RequestId);
}

void ULlamaCppSubsystem::SwitchCharacter(
	const FString& SystemPrompt,
	const TArray<FChatMessage>& FewShotExamples,
	const FString& InitialUserPrompt)
{
	if (!InferenceThread)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Inference thread not initialized"));
		return;
	}

	FLlamaCommand Command;
	Command.Type = ELlamaCommandType::SwitchCharacter;
	Command.SystemPrompt = SystemPrompt;
	Command.FewShotExamples = FewShotExamples;
	Command.UserPrompt = InitialUserPrompt;

	const uint32 RequestId = InferenceThread->QueueCommand(Command);
	UE_LOG(LogLlamaRunner, Display,
		TEXT("Queued character switch request %u"), RequestId);
}

void ULlamaCppSubsystem::ClearChatHistory(const bool bReseed) const
{
	if (!InferenceThread)
	{
		return;
	}

	FLlamaCommand Command;
	Command.Type = ELlamaCommandType::ClearHistory;
	Command.bReseedOnClear = bReseed;

	InferenceThread->QueueCommand(Command);
}

bool ULlamaCppSubsystem::IsProcessing() const
{
	return InferenceThread && InferenceThread->IsProcessing();
}