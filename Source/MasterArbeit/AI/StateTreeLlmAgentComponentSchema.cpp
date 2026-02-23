// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTreeLlmAgentComponentSchema.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "StateTreeExecutionContext.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "Tasks/StateTreeAITask.h"

UStateTreeLlmAgentComponentSchema::UStateTreeLlmAgentComponentSchema(const FObjectInitializer& ObjectInitializer)
	: AIControllerClass(AAIController::StaticClass())
{
	check(ContextDataDescs.Num() == 1 && ContextDataDescs[0].Struct == AActor::StaticClass());
	ContextActorClass = APawn::StaticClass();
	ContextDataDescs[0].Struct = ContextActorClass.Get();
	ContextDataDescs.Emplace(TEXT("AIController"), AIControllerClass.Get(),
		FGuid(0x0D409243, 0x424A1026, 0x22F68B81, 0x16CCAEF1));
}

void UStateTreeLlmAgentComponentSchema::PostLoad()
{
	Super::PostLoad();
	ContextDataDescs[1].Struct = AIControllerClass.Get();
}

bool UStateTreeLlmAgentComponentSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	return Super::IsStructAllowed(InScriptStruct)
		|| InScriptStruct->IsChildOf(FStateTreeAITaskBase::StaticStruct())
		|| InScriptStruct->IsChildOf(FStateTreeAIConditionBase::StaticStruct());
}

bool UStateTreeLlmAgentComponentSchema::SetContextRequirements(UBrainComponent& BrainComponent,
	FStateTreeExecutionContext& Context, bool bLogErrors)
{
	if (!Context.IsValid())
	{
		return false;
	}

	const FName AIControllerName(TEXT("AIController"));
	Context.SetContextDataByName(AIControllerName, FStateTreeDataView(BrainComponent.GetAIOwner()));

	/*const FName SystemPromptName(TEXT("SystemPrompt"));
	Context.SetContextDataByName(SystemPromptName, FStateTreeDataView());*/

	return Super::SetContextRequirements(BrainComponent, Context, bLogErrors);
}

#if WITH_EDITOR
void UStateTreeLlmAgentComponentSchema::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (FProperty* Property = PropertyChangedEvent.Property)
	{
		if (Property->GetOwnerClass() == UStateTreeLlmAgentComponentSchema::StaticClass())
		{
			if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UStateTreeLlmAgentComponentSchema,
				AIControllerClass))
			{
				ContextDataDescs[1].Struct = AIControllerClass.Get();
			}
			/*if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UStateTreeLlmAgentComponentSchema,
			                                                    SystemPromptDataAsset))
			{
				ContextDataDescs[2].Struct = USystemPromptDataAsset::StaticClass();
			}*/
		}
	}
}
#endif