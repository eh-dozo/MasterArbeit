// Fill out your copyright notice in the Description page of Project Settings.

#include "LlamaCppSubsystem.h"

//THIRD_PARTY_INCLUDES_START
#include "llama.h"
//THIRD_PARTY_INCLUDES_END
#include "Misc/FileHelper.h"
#include "Async/Async.h"

DEFINE_LOG_CATEGORY_STATIC(LogLlamaRunner, Log, All);

bool ULlamaCppSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void ULlamaCppSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	llama_log_set([](const enum ggml_log_level LlamaLogLevel, const char* LlamaLogText, void*)
	{
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

	ggml_backend_load_all();

	llama_model_params ModelParams = llama_model_default_params();
	ModelParams.n_gpu_layers = CommonConfig.GraphicalLayers;

	Model.reset(llama_model_load_from_file(TCHAR_TO_UTF8(*CommonConfig.ModelPath.FilePath), ModelParams));
	if (Model.get())
	{
		UE_LOG(LogLlamaRunner, Display, TEXT("Llama model loaded."));

		Vocab = llama_model_get_vocab(Model.get());

		llama_context_params ContextParams = llama_context_default_params();
		ContextParams.n_ctx = CommonConfig.ContextSize;
		ContextParams.n_batch = CommonConfig.Batches;

		Context.reset(llama_init_from_model(Model.get(), ContextParams));
		if (Context.get())
		{
			FString GrammarContent;
			if (FFileHelper::LoadFileToString(GrammarContent, *SamplerConfig.GrammarPath.FilePath))
			{
				GrammarSampler.reset(llama_sampler_init_grammar(
					Vocab,
					TCHAR_TO_UTF8(*GrammarContent),
					"root"));

				if (GrammarSampler.get())
				{
					const uint32 InitialSeed = FMath::Rand();

					// CHAIN ORDER MATTERS !!
					// https://github.com/ggml-org/llama.cpp/pull/3841
					// https://www.reddit.com/r/LocalLLaMA/comments/17vonjo/your_settings_are_probably_hurting_your_model_why/
					Sampler.reset(llama_sampler_chain_init(llama_sampler_chain_default_params()));

					llama_sampler_chain_add(Sampler.get(), GrammarSampler.get());

					if (SamplerConfig.bUseRepetitionPenalty)
					{
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_penalties(
							                        SamplerConfig.PenaltyLastN,
							                        SamplerConfig.PenaltyRepeat,
							                        SamplerConfig.PenaltyFrequency,
							                        SamplerConfig.PenaltyPresent));
					}

					switch (SamplerConfig.DecodingMethod)
					{
					case Traditional:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_top_p(SamplerConfig.TopP, 1));
						if (SamplerConfig.bUseTopKFiltering)
						{
							llama_sampler_chain_add(Sampler.get(), llama_sampler_init_top_k(SamplerConfig.TopK));
						}
						break;
					case MinP:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_min_p(SamplerConfig.MinP, 1));
						break;
					default:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_min_p(SamplerConfig.MinP, 1));
					}

					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_temp(SamplerConfig.Temperature));

					switch (SamplerConfig.SamplerMethod)
					{
					case Dist:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_dist(InitialSeed));
						break;
					case Greedy:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_greedy());
						break;
					case Mirostat:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_mirostat_v2(
							                        InitialSeed,
							                        SamplerConfig.Tau,
							                        SamplerConfig.Eta));
						break;
					default:
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_dist(InitialSeed));
						break;
					}

					UE_LOG(LogLlamaRunner, Display, TEXT("Llama subsystem initialized."));
				}
				else
				{
					UE_LOG(LogLlamaRunner, Error, TEXT("Failed to create grammar sampler!"));
				}
			}
			else
			{
				UE_LOG(LogLlamaRunner, Error, TEXT("Failed to load grammar file!"));
			}
		}
		else
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Failed to create the llama context!"));
		}
	}
	else
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Unable to load the llama model!"));
	}
}

void ULlamaCppSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ClearChatMessages();

	if (Sampler.get())
	{
		// not calling llama_sampler_free since llama_sampler_chain_add was used (c.f. llama.h line 1128)
		Sampler.release();
		Sampler.reset();
	}

	if (Context.get())
	{
		llama_free(Context.release());
		Context.reset();
	}

	if (Model.get())
	{
		llama_model_free(Model.release());
		Model.reset();
	}

	Vocab = nullptr;
}

void ULlamaCppSubsystem::AsyncProcessUserPrompt(const FString& UserPrompt, const FString& SystemPrompt = TEXT(""))
{
	bIsGenerating = true;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, UserPrompt, SystemPrompt]()
	{
		if (!Model.get() || !Context.get())
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Llama model or context is not initialized!"));
			return;
		}

		if (UserPrompt.IsEmpty())
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("Empty user prompt!"));
			return;
		}

		llama_kv_cache_clear(Context.get());

		if (Sampler.get())
		{
			llama_sampler_reset(Sampler.get());
		}

		TArray<uint8> Formatted;
		const int ContextSize = llama_n_ctx(Context.get());
		Formatted.SetNumUninitialized(ContextSize);

		// TODO: refactor later to allow pre-defined template names
		// https://github.com/ggml-org/llama.cpp/wiki/Templates-supported-by-llama_chat_apply_template
		const char* Template = llama_model_chat_template(Model.get(), nullptr);

		if (ChatMessages.IsEmpty() && !SystemPrompt.IsEmpty())
		{
			// TCHAR_TO_UTF8 cause dangling pointer if no _strdup to copy and allocate
			ChatMessages.Push({"system", _strdup(TCHAR_TO_UTF8(*SystemPrompt))});
		}

		// TCHAR_TO_UTF8 cause dangling pointer if no _strdup to copy and allocate
		ChatMessages.Push({"user", _strdup(TCHAR_TO_UTF8(*UserPrompt))});

		int NewLength = llama_chat_apply_template(
			Template,
			ChatMessages.GetData(),
			ChatMessages.Num(),
			true,
			reinterpret_cast<char*>(Formatted.GetData()),
			Formatted.Num());

		if (NewLength > Formatted.Num())
		{
			Formatted.SetNumUninitialized(NewLength);
			NewLength = llama_chat_apply_template(
				Template,
				ChatMessages.GetData(),
				ChatMessages.Num(),
				true,
				reinterpret_cast<char*>(Formatted.GetData()),
				Formatted.Num());
		}
		if (NewLength < 0)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Failed to apply chat template to new length!"));
			return;
		}

		FString Prompt(NewLength, reinterpret_cast<const char*>(Formatted.GetData()));

		FString Response = Generate(Prompt);

		if (Response.IsEmpty())
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("Generated empty response, retrying with clean state"));
			llama_kv_cache_clear(Context.get());
			Response = Generate(Prompt);
		}

		// TCHAR_TO_UTF8 cause dangling pointer if no _strdup to copy and allocate
		ChatMessages.Push({"assistant", _strdup(TCHAR_TO_UTF8(*Response))});

#if !UE_BUILD_SHIPPING
		for (auto [role, content] : ChatMessages)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Role: %hs | Content: %hs"), role, content);
		}
#endif

		OnInferenceComplete.Broadcast(ChatMessages.Last().content);
	});
}

FString ULlamaCppSubsystem::Generate(const FString& Prompt)
{
	FString Response;
	const bool IsFirstGeneration = llama_get_kv_cache_used_cells(Context.get()) == 0;

	const int AmountPromptTokens = -llama_tokenize(
		Vocab,
		TCHAR_TO_UTF8(*Prompt),
		Prompt.Len(),
		nullptr,
		0,
		IsFirstGeneration,
		true);

	if (AmountPromptTokens <= 0)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to count tokens in prompt!"));
		return Response;
	}

	TArray<llama_token> PromptTokens;
	PromptTokens.SetNum(AmountPromptTokens);

	if (llama_tokenize(
		Vocab,
		TCHAR_TO_UTF8(*Prompt),
		Prompt.Len(),
		PromptTokens.GetData(),
		PromptTokens.Num(),
		IsFirstGeneration,
		true) < 0)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Failed to tokenize the prompt!"));
		GGML_ABORT("Failed to tokenize the prompt\n");
	}

	const int BatchSize = 32;
	int TokensProcessed = 0;

	while (TokensProcessed < PromptTokens.Num())
	{
		int TokensToProcess = FMath::Min(BatchSize, PromptTokens.Num() - TokensProcessed);
		llama_batch Batch = llama_batch_get_one(PromptTokens.GetData() + TokensProcessed, TokensToProcess);

		int ContextSpace = llama_n_ctx(Context.get());
		int ContextSpaceUsed = llama_get_kv_cache_used_cells(Context.get());

		if (ContextSpaceUsed + Batch.n_tokens > ContextSpace)
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("Context size exceeded during prompt processing!"))
			break;
		}

		int DecodeStatus = llama_decode(Context.get(), Batch);

		if (DecodeStatus == 1)
		{
			UE_LOG(LogLlamaRunner, Error,
			       TEXT(
				       "Decode status: Could not find a KV slot for the batch (try reducing the size of the batch or increase the context)!"
			       ));
			GGML_ABORT(
				"Decode status: Could not find a KV slot for the batch (try reducing the size of the batch or increase the context)!\n");
		}
		if (DecodeStatus < 0)
		{
			UE_LOG(LogLlamaRunner, Error,
			       TEXT("Decode status: The KV cache state is restored to the state before this call!"));
			GGML_ABORT("Decode status: The KV cache state is restored to the state before this call!\n");
		}

		TokensProcessed += TokensToProcess;
	}

	const int MaxNewTokens = 1024;
	int GeneratedTokens = 0;
	/*int LastLoggedPercent = -1; // Track the last logged percentage to avoid spam

	UE_LOG(LogLlamaRunner, Display, TEXT("Starting generation: 0%% (0/%d tokens)"), MaxNewTokens);*/

	llama_token NewTokenId = 0;
	while (GeneratedTokens < MaxNewTokens)
	{
		int ContextSpace = llama_n_ctx(Context.get());
		int ContextSpaceUsed = llama_get_kv_cache_used_cells(Context.get());

		if (ContextSpaceUsed + 1 > ContextSpace)
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("Context size exceeded during generation!"));
			break;
		}

		try
		{
			NewTokenId = llama_sampler_sample(Sampler.get(), Context.get(), -1);
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Exception in llama_sampler_sample: %hs"), e.what());
			break;
		}
		catch (...)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Unknown exception in llama_sampler_sample"));
			break;
		}

		if (llama_vocab_is_eog(Vocab, NewTokenId))
		{
			break;
		}

		char Buf[256];
		int TokenToPiece = llama_token_to_piece(
			Vocab,
			NewTokenId,
			Buf,
			sizeof(Buf),
			0,
			true);

		if (TokenToPiece < 0)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Failed to convert token to piece!"));
			GGML_ABORT("Failed to convert token to piece\n");
		}

		std::string Piece(Buf, TokenToPiece);
		Response.Append(Piece.c_str());

		llama_batch Batch = llama_batch_get_one(&NewTokenId, 1);
		int DecodeStatus = llama_decode(Context.get(), Batch);

		if (DecodeStatus != 0)
		{
			UE_LOG(LogLlamaRunner, Error, TEXT("Decode error during generation: %d"), DecodeStatus);
			break;
		}

		GeneratedTokens++;

		// Log generation progress at 10% intervals
		/*int CurrentPercent = (GeneratedTokens * 100) / MaxNewTokens;
		if ((CurrentPercent / 10) * 10 != (LastLoggedPercent / 10) * 10 || CurrentPercent == 100)
		{
			int PercentToLog = (CurrentPercent / 10) * 10;
			if (CurrentPercent == 100 || PercentToLog > LastLoggedPercent)
			{
				UE_LOG(LogLlamaRunner, Display, TEXT("Generation progress: %d%% (%d/%d tokens)"), 
					PercentToLog, GeneratedTokens, MaxNewTokens);
				LastLoggedPercent = PercentToLog;
			}
		}*/

		if (GeneratedTokens > 5 && Response.IsEmpty())
		{
			UE_LOG(LogLlamaRunner, Warning, TEXT("Generated multiple tokens but response is still empty"));
			break;
		}
	}

	bIsGenerating = false;

	return Response;
}

void ULlamaCppSubsystem::ClearChatMessages()
{
	for (const llama_chat_message& ChatMessage : ChatMessages)
	{
		free(const_cast<char*>(ChatMessage.content));
	}

	ChatMessages.Empty();
}

void ULlamaCppSubsystem::ClearChatHistory()
{
	if (bIsGenerating)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Model is still running, aborting history clearance."));
		return;
	}

	ClearChatMessages();

	if (Sampler.get())
	{
		Sampler.release();
		Sampler.reset();
	}

	if (Context.get())
	{
		llama_kv_cache_clear(Context.get());

		llama_context_params ContextParams = llama_context_default_params();
		ContextParams.n_ctx = CommonConfig.ContextSize;
		ContextParams.n_batch = CommonConfig.Batches;

		llama_free(Context.release());
		Context.reset(llama_init_from_model(Model.get(), ContextParams));
	}

	if (Context.get() && Model.get())
	{
		FString GrammarContent;
		if (FFileHelper::LoadFileToString(GrammarContent, *SamplerConfig.GrammarPath.FilePath))
		{
			GrammarSampler.reset(llama_sampler_init_grammar(
				Vocab,
				TCHAR_TO_UTF8(*GrammarContent),
				"root"));

			if (GrammarSampler.get())
			{
				const uint32 TimeSeed = FDateTime::Now().GetTicks() & 0xFFFFFFFF;
				const uint32 RandomComponent = FMath::Rand();
				const uint32 NewSeed = TimeSeed ^ RandomComponent;

				Sampler.reset(llama_sampler_chain_init(llama_sampler_chain_default_params()));

				llama_sampler_chain_add(Sampler.get(), GrammarSampler.get());

				if (SamplerConfig.bUseRepetitionPenalty)
				{
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_penalties(
						                        SamplerConfig.PenaltyLastN,
						                        SamplerConfig.PenaltyRepeat,
						                        SamplerConfig.PenaltyFrequency,
						                        SamplerConfig.PenaltyPresent));
				}

				switch (SamplerConfig.DecodingMethod)
				{
				case Traditional:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_top_p(SamplerConfig.TopP, 1));
					if (SamplerConfig.bUseTopKFiltering)
					{
						llama_sampler_chain_add(Sampler.get(), llama_sampler_init_top_k(SamplerConfig.TopK));
					}
					break;
				case MinP:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_min_p(SamplerConfig.MinP, 1));
					break;
				default:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_min_p(SamplerConfig.MinP, 1));
				}

				llama_sampler_chain_add(Sampler.get(), llama_sampler_init_temp(SamplerConfig.Temperature));

				switch (SamplerConfig.SamplerMethod)
				{
				case Dist:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_dist(NewSeed));
					break;
				case Greedy:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_greedy());
					break;
				case Mirostat:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_mirostat_v2(
						                        NewSeed,
						                        SamplerConfig.Tau,
						                        SamplerConfig.Eta));
					break;
				default:
					llama_sampler_chain_add(Sampler.get(), llama_sampler_init_dist(NewSeed));
					break;
				}

				UE_LOG(LogLlamaRunner, Display, TEXT("Context and sampler reinitialized successfully"));
			}
			else
			{
				UE_LOG(LogLlamaRunner, Warning, TEXT("GrammarSampler is null, cannot recreate sampler chain"));
			}
		}
	}

	UE_LOG(LogLlamaRunner, Display, TEXT("Chat history has been cleared successfully"));
}

void ULlamaCppSubsystem::AddChatHistory(const TArray<FChatMessage>& ChatHistory,
                                             const FString& SystemPrompt,
                                             bool bClearHistoryFirst)
{
	if (bIsGenerating)
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Model is still running, aborting: clear and add chat history."));
		return;
	}

	if (ChatHistory.Num() == 0)
	{
		UE_LOG(LogLlamaRunner, Warning, TEXT("Empty chat history provided."));
		return;
	}

	if (bClearHistoryFirst)
	{
		ClearChatHistory();
	}

	bool bExpectingUser = true;

	if (ChatMessages.IsEmpty() && !SystemPrompt.IsEmpty())
	{
		ChatMessages.Push({"system", _strdup(TCHAR_TO_UTF8(*SystemPrompt))});
	}

	for (int i = 0; i < ChatHistory.Num(); ++i)
	{
		const FString& ChatRole = *StaticEnum<EChatRole>()->GetNameStringByValue(ChatHistory[i].ChatRole);

		if (bExpectingUser)
		{
			if (ChatHistory[i].ChatRole != EChatRole::User)
			{
				UE_LOG(LogLlamaRunner, Error,
				       TEXT(
					       "Expected 'user' role at index %d but got '%s'. Chat history must alternate between 'user' and 'assistant' starting with 'user'."
				       ),
				       i, *ChatRole)

				ClearChatMessages();
				return;
			}
			bExpectingUser = false;
		}
		else
		{
			if (ChatHistory[i].ChatRole != EChatRole::Assistant)
			{
				UE_LOG(LogLlamaRunner, Error,
				       TEXT(
					       "Expected 'assistant' role at index %d but got '%s'. Chat history must alternate between 'user' and 'assistant' starting with 'user'."
				       ),
				       i, *ChatRole)

				ClearChatMessages();
				return;
			}
			bExpectingUser = true;
		}

		const FString& ChatRoleLower = *ChatRole.ToLower();
		const FString& ChatContent = *ChatHistory[i].Content;

#if !UE_BUILD_SHIPPING
		UE_LOG(LogLlamaRunner, Warning, TEXT("Loaded - Role: %s | Content: %s"), *ChatRoleLower, *ChatContent);
#endif

		ChatMessages.Push({
			_strdup(TCHAR_TO_UTF8(*ChatRole.ToLower())),
			_strdup(TCHAR_TO_UTF8(*ChatContent))
		});
	}

	UE_LOG(LogLlamaRunner, Display, TEXT("Successfully loaded %d chat messages from history."), ChatMessages.Num());
}

void ULlamaCppSubsystem::SetSamplerConfig(FLlamaCppSubsystemSamplerConfig NewConfig)
{
	if (!bIsGenerating)
	{
		SamplerConfig = NewConfig;

		ClearChatHistory();
	}
	else
	{
		UE_LOG(LogLlamaRunner, Error, TEXT("Model is still running, aborting new sampler config setting."));
	}
}
