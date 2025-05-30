// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTreeLlmAgentComponent.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLlmAgentComponentSchema.h"


TSubclassOf<UStateTreeSchema> UStateTreeLlmAgentComponent::GetSchema() const
{
	return UStateTreeLlmAgentComponentSchema::StaticClass();
}

bool UStateTreeLlmAgentComponent::SetContextRequirements(FStateTreeExecutionContext& Context, bool bLogErrors)
{
	Context.SetCollectExternalDataCallback(FOnCollectStateTreeExternalData::CreateUObject(this, &UStateTreeLlmAgentComponent::CollectExternalData));
	return UStateTreeLlmAgentComponentSchema::SetContextRequirements(*this, Context, bLogErrors);
}
