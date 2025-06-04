// Fill out your copyright notice in the Description page of Project Settings.


#include "STTConstructCharacterPrompt.h"
#include "StateTreeExecutionContext.h"

#define LOCTEXT_NAMESPACE "StateTree"

EStateTreeRunStatus FStateTreeCharacterPromptTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.SystemPromptDataAsset != NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *InstanceData.SystemPromptDataAsset->SystemPrompt.ToString());
	}
	
	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FStateTreeCharacterPromptTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	check(InstanceData);
	
	const FText Format = (Formatting == EStateTreeNodeFormatting::RichText)
			? LOCTEXT("ConstructCharacterPromptRich", "<b>Character Prompt</> \"{Text}\"")
			: LOCTEXT("ConstructCharacterPrompt", "Character Prompt \"{Text}\"");

	return FText::FormatNamed(Format,
		TEXT("Text"), LOCTEXT("The prompt", "The prompt")); //TODO: find better description
}
#endif

#undef LOCTEXT_NAMESPACE