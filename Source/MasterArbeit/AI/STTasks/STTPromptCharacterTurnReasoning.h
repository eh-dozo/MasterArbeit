#pragma once
#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "MasterArbeit/AI/Memory/AgentMemoryDataAssets.h"
#include "STTPromptCharacterTurnReasoning.generated.h"

enum class EStateTreeRunStatus : uint8;
struct FStateTreeTransitionResult;

USTRUCT()
struct FStateTreePromptChracterTurnReasoningTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<USystemPromptDataAsset> SystemPromptDataAsset = nullptr;

	UPROPERTY(EditAnywhere, Category=Input)
	TSoftObjectPtr<UDataTable> ChatHistoryDataTable = nullptr;
};

USTRUCT(meta=(DisplayName="Prompt Character Turn Reasoning"))
struct MASTERARBEIT_API FStateTreePromptCharacterTurnReasoningTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreePromptChracterTurnReasoningTaskInstanceData;

	FStateTreePromptCharacterTurnReasoningTask() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;


	#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;

	virtual FName GetIconName() const override
	{
		return FName("StateTreeEditorStyle|Node.Text");
	}

	virtual FColor GetIconColor() const override
	{
		return UE::StateTree::Colors::Grey;
	}
	#endif
};