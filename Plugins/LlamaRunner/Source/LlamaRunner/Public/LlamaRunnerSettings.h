#pragma once

#include "CoreMinimal.h"
#include "ggml.h"
#include "Engine/DeveloperSettings.h"
#include "LlamaRunnerSettings.generated.h"

UENUM(meta=(ToolTip=
	"Traditional decoding refer to Top-P and Top-K sampling, the other methods are self-explanatory"
))
enum ELlamaDecodingMethod
{
	Traditional,
	MinP UMETA(DisplayName="min-p"),
};

UENUM()
enum ELlamaSamplerMethod
{
	Greedy, //will attempt to output the largest response possible!
	Dist,
	Mirostat
};

UENUM(meta=(Experimental, ToolTip="Used to configure the LlamaRunner llama.cpp cpu process priority."))
enum EGgmlSchedulePriority
{
	Scheduling_Priority_Normal UMETA(DisplayName="Normal"),
	Scheduling_Priority_Medium UMETA(DisplayName="Medium"),
	Scheduling_Priority_High UMETA(DisplayName="High"),
	Scheduling_Priority_Runtime UMETA(DisplayName="Runtime"),
};

UCLASS(Hidden, HideDropdown)
class LLAMARUNNER_API ULlamaUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static ggml_sched_priority ConvertToGgmlPriority(const EGgmlSchedulePriority UnrealPriority)
	{
		switch (UnrealPriority)
		{
		case Scheduling_Priority_Normal:
			return GGML_SCHED_PRIO_NORMAL;
		case Scheduling_Priority_Medium:
			return GGML_SCHED_PRIO_MEDIUM;
		case Scheduling_Priority_High:
			return GGML_SCHED_PRIO_HIGH;
		case Scheduling_Priority_Runtime:
			return GGML_SCHED_PRIO_REALTIME;
		default:
			return GGML_SCHED_PRIO_REALTIME;
		}
	}
};

UCLASS(Config=Game, ProjectUserConfig, DisplayName="Llama Runner Settings")
class LLAMARUNNER_API UDSLlamaRunnerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return "Plugins"; }
	virtual FText GetSectionText() const override { return FText::FromString("Llama Runner"); }
	virtual FText GetSectionDescription() const override { return FText::FromString("The model path MUST be specified or the plugin won't work.\nAll the other settings can be left to default if nothing specific is required to use the plugin."); }

	/*
	 * ----------------------------------------------------------|
	 *						GENERAL SETTINGS					 *
	 * ----------------------------------------------------------|
	 */

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ToolTip="Must point to a .gguf file"),
		Category="General",
		DisplayName="Path to model file")
	FFilePath ModelPath = FFilePath("");

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ClampMin="-1",
			ClampMax="32768",
			ToolTip="WARNING: should be a power of 2"),
		Category="General",
		DisplayName="Model context size")
	int32 ContextSize = -1;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ClampMin="-1",
			ClampMax="8192",
			ToolTip="WARNING: should be a power of 2"),
		Category="General",
		DisplayName="Tokens processed per batch")
	int32 Batches = -1;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite,
		Category="General",
		DisplayName="Don't log inference performances")
	bool bIsNoPerf = false;

	/*
	 * ----------------------------------------------------------|
	 *						SAMPLER SETTINGS					 *
	 * ----------------------------------------------------------|
	 */

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ToolTip="Must point to a .gbnf file"),
		Category="Sampler",
		DisplayName="Path to grammar file")
	FFilePath GrammarPath = FFilePath("");

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ToolTip="Note: will attempt to output the largest response possible!"),
		Category="Sampler",
		DisplayName="Sampler method")
	TEnumAsByte<ELlamaSamplerMethod> SamplerMethod = Dist;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="SamplerMethod != Mirostat",
			ToolTip="Note: not available for Mirostat"),
		Category="Sampler",
		DisplayName="Decoding method")
	TEnumAsByte<ELlamaDecodingMethod> DecodingMethod = MinP;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ClampMin="0",
			ClampMax="2"),
		Category="Sampler")
	float Temperature = 0.8f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="DecodingMethod == MinP && SamplerMethod != Mirostat",
			ClampMin="0.0",
			ClampMax="1.0",
			ToolTip=
			"Note: a value of 0 disables min-p filtering (considers all tokens) and a value of 1 is extremely restrictive. Recommended values usually are between 0.05 and 0.1"
		),
		Category="Sampler",
		DisplayName="min-p")
	float MinPValue = 0.05f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition=
			"DecodingMethod == Traditional && SamplerMethod != Mirostat",
			ClampMin="0.0",
			ClampMax="1.0",
			ToolTip=
			"Note: a value of 0 disables top-p filtering and a value of 1 allows every tokens. Recommended values usually are between 0.85 and 0.95"
		),
		Category="Sampler",
		DisplayName="top-p")
	float TopP = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite,
		meta=(
			EditCondition=
			"DecodingMethod == Traditional && SamplerMethod != Mirostat"),
		Category="Sampler",
		DisplayName="[DEBUG] Use top-k filtering")
	bool bUseTopKFiltering = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition=
			"bUseTopKFiltering && DecodingMethod == Traditional && SamplerMethod != Mirostat"
			,
			ClampMin="0",
			ClampMax="1000",
			ToolTip=
			"Note: since any models has its own vocabulary length, max is arbitrarly set to 1000. A value of 0 disables top-k filtering (considers all tokens) and a value of n allows all top n tokens in the vocabulary. Recommended values usually are between 35 and 50. Using top-k is nowadays not recommended anymore."
		),
		Category="Sampler",
		DisplayName="top-k")
	int32 TopK = 0;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition=
			"SamplerMethod == Mirostat",
			ClampMin="0.1",
			ClampMax="10",
			ToolTip=
			"Note: A higher value corresponds to more surprising or less predictable text, while a lower value corresponds to less surprising or more predictable text."
		),
		Category="Sampler",
		DisplayName="tau")
	float Tau = 5.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition=
			"SamplerMethod == Mirostat",
			ClampMin="0.05",
			ClampMax="0.2",
			ToolTip=
			"Note: Eta is very sensitive, controls the perplexity. Not many documentation can be found on it, so the range is arbitrary based on online reviews."
		),
		Category="Sampler",
		DisplayName="eta")
	float Eta = 0.1f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite,
		Category="Sampler",
		DisplayName="Use repetition penalty")
	bool bUseRepetitionPenalty = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="bUseRepetitionPenalty",
			ClampMin="-1",
			ClampMax="1000"),
		Category="Sampler",
		DisplayName="Repetition Penalty: last-n")
	int32 PenaltyLastN = 64;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="bUseRepetitionPenalty",
			ClampMin="1.0",
			ClampMax="1.2"),
		Category="Sampler",
		DisplayName="Repetition Penalty: repeat")
	float PenaltyRepeat = 1.1f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="bUseRepetitionPenalty",
			ClampMin="1.0",
			ClampMax="1.2"),
		Category="Sampler",
		DisplayName="Repetition Penalty: frequency")
	float PenaltyFrequency = 0.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="bUseRepetitionPenalty",
			ClampMin="1.0",
			ClampMax="1.2"),
		Category="Sampler",
		DisplayName="Repetition Penalty: present")
	float PenaltyPresent = 0.0f;

	/*
	 * ----------------------------------------------------------|
	 *						HARDWARE SETTINGS					 *
	 * ----------------------------------------------------------|
	 */

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			ToolTip=
			"Should the amount of threads for the llama.cpp be processed:\n(0) automatically: using the common.h library\n(1) manually: setting a {ReservedCores} parameter for the operation:\nconst int32 NumLogicalCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();\nconst int32 LlamaNumThreads = FMath::Max(1, NumLogicalCores - ReservedCores);"
		),
		Category="Hardware",
		DisplayName="Automatically determine the amount of threads to use for inferences.")
	bool bUseCommonThreadEstimation = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			EditCondition="!bUseCommonThreadEstimation",
			ClampMin="2",
			ClampMax="16",
			ToolTip="WARNING: Use with cautious!!!\nShould be set based on the CPU specs YOUR computer have."),
		Category="Hardware",
		DisplayName="Reserved CPU cores for Unreal Engine (NOT LlamaRunner)")
	int32 ReservedCores = 2;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta=(
			Experimental,
			ToolTip="Used to configure the LlamaRunner llama.cpp cpu process priority."),
		Category="Hardware",
		DisplayName="GGML process schedule priority")
	TEnumAsByte<EGgmlSchedulePriority> GgmlSchedulePriority = Scheduling_Priority_Runtime;
};
