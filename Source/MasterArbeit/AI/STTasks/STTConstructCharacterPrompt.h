// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "MasterArbeit/AI/Memory/AgentMemoryDataAssets.h"
#include "STTConstructCharacterPrompt.generated.h"

enum class EStateTreeRunStatus : uint8;
struct FStateTreeTransitionResult;

USTRUCT()
struct MASTERARBEIT_API FStateTreeConstructCharacterPromptTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<USystemPromptDataAsset> SystemPromptDataAsset = nullptr;
};

USTRUCT(meta=(DisplayName="Character Prompt"))
struct MASTERARBEIT_API FStateTreeCharacterPromptTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeConstructCharacterPromptTaskInstanceData;

	FStateTreeCharacterPromptTask() = default;

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