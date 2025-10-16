// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//THIRD_PARTY_INCLUDES_START
#include "llama.h"
#include "llama-cpp.h"
//THIRD_PARTY_INCLUDES_END
#include "CoreMinimal.h"
#include "LlamaRunnerSettings.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LlamaCppSubsystem.generated.h"

/*UENUM(meta=(ToolTip=
	"Traditional decoding refer to Top-P and Top-K sampling, the other methods are self-explanatory"
))
enum ELlamaDecodingMethod
{
	Traditional,
	MinP
};

UENUM()
enum ELlamaSamplerMethod
{
	Greedy,
	Dist,
	Mirostat
};*/

UENUM(BlueprintType)
enum EChatRole
{
	System,
	User,
	Assistant
};

USTRUCT(BlueprintType)
struct FLamaCppSubsystemCommonConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner")
	FFilePath ModelPath = FFilePath("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		ClampMin="-1",
		ClampMax="8192",
		ToolTip="WARNING: should be a power of 2"))
	int32 ContextSize = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		ClampMin="-1",
		ClampMax="8192",
		ToolTip="WARNING: should be a power of 2"))
	int32 Batches = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		ClampMin="-1",
		ClampMax="99",
		ToolTip="WARNING: be carefull with your hardware, high values will require more GPU"))
	int32 GraphicalLayers = -1;
};

USTRUCT(BlueprintType)
struct FLlamaCppSubsystemSamplerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner")
	FFilePath GrammarPath = FFilePath("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		ToolTip="WARNING: Greedy will attempt to output the largest response possible, which might not always be ideal!"))
	TEnumAsByte<ELlamaSamplerMethod> SamplerMethod = Dist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition="SamplerMethod!=Mirostat"))
	TEnumAsByte<ELlamaDecodingMethod> DecodingMethod = ELlamaDecodingMethod::MinP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		ClampMin="0",
		ClampMax="2"))
	float Temperature = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition="DecodingMethod==ELlamaDecodingMethod::MinP && SamplerMethod!=Mirostat",
		ClampMin="0.0",
		ClampMax="1.0",
		ToolTip=
		"Note: a value of 0 disables min-p filtering (considers all tokens) and a value of 1 is extremely restrictive. Recommended values usually are between 0.05 and 0.1"
	))
	float MinP = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition=
		"DecodingMethod==Traditional && SamplerMethod!=Mirostat",
		ClampMin="0.0",
		ClampMax="1.0",
		ToolTip=
		"Note: a value of 0 disables top-p filtering and a value of 1 allows every tokens. Recommended values usually are between 0.85 and 0.95"
	))
	float TopP = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner",
		DisplayName="[DEBUG] Use top-k filtering",
		meta=(
			EditCondition=
			"DecodingMethod==Traditional && SamplerMethod!=Mirostat"))
	bool bUseTopKFiltering = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition=
		"bUseTopKFiltering && DecodingMethod==Traditional && SamplerMethod!=Mirostat"
		,
		ClampMin="0",
		ClampMax="1000",
		ToolTip=
		"Note: since any models has its own vocabulary length, max is arbitrarly set to 1000. A value of 0 disables top-k filtering (considers all tokens) and a value of n allows all top n tokens in the vocabulary. Recommended values usually are between 35 and 50. Using top-k is nowadays not recommended anymore."
	))
	int32 TopK = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition=
		"SamplerMethod==Mirostat",
		ClampMin="0.1",
		ClampMax="10",
		ToolTip=
		"Note: A higher value corresponds to more surprising or less predictable text, while a lower value corresponds to less surprising or more predictable text."
	))
	float Tau = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition=
		"SamplerMethod==Mirostat",
		ClampMin="0.05",
		ClampMax="0.2",
		ToolTip=
		"Note: Eta is very sensitive, controls the perplexity. Not many documentation can be found on it, so the range is arbitrary based on online reviews."
	))
	float Eta = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", DisplayName="Use repetition penalty")
	bool bUseRepetitionPenalty = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition="bUseRepetitionPenalty",
		ClampMin="-1",
		ClampMax="1000"))
	int32 PenaltyLastN = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition="bUseRepetitionPenalty",
		ClampMin="1.0",
		ClampMax="1.2"))
	float PenaltyRepeat = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition="bUseRepetitionPenalty",
		ClampMin="1.0",
		ClampMax="1.2"))
	float PenaltyFrequency = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LlamaRunner", meta=(
		EditCondition="bUseRepetitionPenalty",
		ClampMin="1.0",
		ClampMax="1.2"))
	float PenaltyPresent = 0.0f;
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInferenceCompleteDelegate, const FString&, Response);

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class LLAMARUNNER_API ULlamaCppSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LlamaRunner")
	FLamaCppSubsystemCommonConfig CommonConfig;

	UPROPERTY(EditAnywhere, BlueprintSetter=SetSamplerConfig, Category="LlamaRunner")
	FLlamaCppSubsystemSamplerConfig SamplerConfig;
	
	UPROPERTY(BlueprintAssignable, Category = "LlamaRunner")
	FOnInferenceCompleteDelegate OnInferenceComplete;

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

	UFUNCTION(BlueprintCallable, Category="LlamaRunner")
	void AsyncProcessUserPrompt(const FString& UserPrompt, const FString& SystemPrompt);

	UFUNCTION(BlueprintCallable, Category="LlamaRunner")
	void ClearChatHistory();

	UFUNCTION(BlueprintCallable, Category="LlamaRunner")
	void AddChatHistory(const TArray<FChatMessage>& ChatHistory, const FString& SystemPrompt, bool bClearHistoryFirst);
	
	UFUNCTION(BlueprintSetter, Category="LlamaRunner")
	void SetSamplerConfig(FLlamaCppSubsystemSamplerConfig NewConfig);

private:
	FString Generate(const FString& Prompt);

	void ClearChatMessages();
};
