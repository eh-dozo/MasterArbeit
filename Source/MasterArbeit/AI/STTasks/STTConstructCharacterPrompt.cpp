// Fill out your copyright notice in the Description page of Project Settings.


#include "STTConstructCharacterPrompt.h"

EStateTreeRunStatus USTTConstructCharacterPrompt::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	//Super::EnterState(Context, Transition);
	UE_LOG(LogTemp, Error, TEXT("%s"), *SystemPromptDataAsset->SystemPrompt.ToString());
	return EStateTreeRunStatus::Succeeded;
}
