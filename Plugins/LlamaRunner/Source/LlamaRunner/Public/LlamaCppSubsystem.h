// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//THIRD_PARTY_INCLUDES_START
#include "llama.h"
#include "llama-cpp.h"
#include  "common.h"
//THIRD_PARTY_INCLUDES_END
#include "CoreMinimal.h"
#include "chat.h"
#include "LlamaRunnerSettings.h"
#include "sampling.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LlamaCppSubsystem.generated.h"

LLAMARUNNER_API DECLARE_LOG_CATEGORY_EXTERN(LogLlamaRunner, Log, All);

UENUM(BlueprintType)
enum EChatRole
{
	System,
	User,
	Assistant
};

USTRUCT(BlueprintType)
struct FChatMessage : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner")
	TEnumAsByte<EChatRole> ChatRole = System;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(MultiLine))
	FString Content;
};

UENUM(BlueprintType)
enum class ELlamaCommandType : uint8
{
	ContinueChat,
	SwitchCharacter,
	ClearHistory,
	Shutdown
};

struct FLlamaCommand
{
	ELlamaCommandType Type;
	FString UserPrompt;
	FString SystemPrompt;
	TArray<FChatMessage> FewShotExamples;
	uint32 RequestId; // for tracking
};

class FLlamaModelState
{
public:
	FLlamaModelState();
	~FLlamaModelState();

	bool Initialize();
	
	void InitializeCommonParams();
	common_params_sampling InitializeSamplerParams() const;
	void InitializeThreadPools();

	void Shutdown();
	
	bool IsValid() const { return Model && Context && CommonSampler; }

	// Callback signature for context warnings during generation
	using FContextWarningCallback = TFunction<void(int32 TokensUsed, int32 TokensTotal, float UsagePercent)>;

	FString Generate(const FContextWarningCallback& WarningCallback = nullptr) const;
	void ClearCache();
	void ResetSampler() const;

	// Context monitoring functions
	int32 GetContextTokensUsed() const;
	int32 GetContextTokensTotal() const { return N_Ctx; }
	int32 GetContextTokensRemaining() const;
	float GetContextUsagePercent() const;
	bool CanFitTokens(int32 TokenCount, int32 MinGenerationBuffer = 512) const;

	void ClearMessages() const { CommonMessages->clear(); }
	void AddChatAndFormat(EChatRole Role, const FString& Content) const;
	size_t GetMessageCount() const { return CommonMessages->size(); }

private:
	const uint32 InitialSamplerSeed = []
	{
		const uint32 TimeSeed = FDateTime::Now().GetTicks() & 0xFFFFFFFF;
		const uint32 RandomComponent = FMath::Rand();
		return TimeSeed ^ RandomComponent;
	}();

	const UDSLlamaRunnerSettings* GeneralSettings = GetDefault<UDSLlamaRunnerSettings>();
	
	llama_model_ptr Model = nullptr;
	llama_context_ptr Context = nullptr;
	common_sampler* CommonSampler = nullptr;
	common_params* CommonParams = nullptr;

	ggml_threadpool* ThreadPool = nullptr;
	ggml_threadpool* ThreadPoolBatch = nullptr;

	std::vector<common_chat_msg>* CommonMessages = nullptr;

	std::vector<llama_token> SessionTokens;
	int32 SessionTokensConsumed = 0;
	int32 N_Ctx;

	friend class FLlamaInferenceThread;
};

class FLlamaInferenceThread : public FRunnable
{
public:
	FLlamaInferenceThread(ULlamaCppSubsystem* InOwner);
	virtual ~FLlamaInferenceThread();

	// FRunnable interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	uint32 QueueCommand(const FLlamaCommand& Command);
	bool IsProcessing() const { return bIsProcessing.load(); }

private:
	const UDSLlamaRunnerSettings* GeneralSettings = GetDefault<UDSLlamaRunnerSettings>();
	
	void ProcessContinueChat(const FLlamaCommand& Command);
	void ProcessSwitchCharacter(const FLlamaCommand& Command);
	void ProcessClearHistory(const FLlamaCommand& Command);

	static void BindLlamaRunnerLogs();
	void LogSamplingAndGenerationPerformances() const;

	ULlamaCppSubsystem* Owner;

	FRunnableThread* Thread;
	std::atomic<bool> bShutdown;
	std::atomic<bool> bIsProcessing;
	FEvent* WakeUpEvent;

	TQueue<FLlamaCommand, EQueueMode::Mpsc> CommandQueue;
	std::atomic<uint32> NextRequestId;

	TUniquePtr<FLlamaModelState> ModelState;

	FString CurrentSystemPrompt;
	TArray<FChatMessage> CurrentFewShots;
	bool bHasActiveConversation = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInferenceCompleteDelegate, const FString&, Response);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnContextWarningDelegate,
	int32, TokensUsed,
	int32, TokensTotal,
	float, UsagePercent
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnContextErrorDelegate,
	int32, TokensUsed,
	int32, TokensTotal,
	int32, TokensRemaining
);

/**
 * 
 */
UCLASS()
class LLAMARUNNER_API ULlamaCppSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	const UDSLlamaRunnerSettings* GeneralSettings = GetDefault<UDSLlamaRunnerSettings>();
	
	UPROPERTY(BlueprintAssignable, Category = "LlamaRunner")
	FOnInferenceCompleteDelegate OnInferenceComplete;

	UPROPERTY(BlueprintAssignable, Category = "LlamaRunner")
	FOnContextWarningDelegate OnContextWarning;

	UPROPERTY(BlueprintAssignable, Category = "LlamaRunner")
	FOnContextErrorDelegate OnContextError;

private:
	llama_model_ptr Model;
	const llama_vocab* Vocab;
	llama_context_ptr Context;
	llama_sampler_ptr Sampler;
	llama_sampler_ptr GrammarSampler;

	TArray<llama_chat_message> ChatMessages;

	bool bIsGenerating;

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="LlamaRunner",
		meta=(DisplayName="Continue Conversation"))
	void ContinueConversation(const FString& UserPrompt) const;

	UFUNCTION(BlueprintCallable, Category="LlamaRunner",
		meta=(DisplayName="Switch Character"))
	void SwitchCharacter(
		const FString& SystemPrompt,
		const TArray<FChatMessage>& FewShotExamples,
		const FString& InitialUserPrompt);

	UFUNCTION(BlueprintCallable, Category="LlamaRunner")
	void ClearChatHistory() const;

	UFUNCTION(BlueprintCallable, Category="LlamaRunner")
	bool IsProcessing() const;

private:
	TUniquePtr<FLlamaInferenceThread> InferenceThread;

	std::atomic<uint32> LastRequestId;

	friend class FLlamaInferenceThread;
};
