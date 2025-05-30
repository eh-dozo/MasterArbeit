// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "MasterArbeit/AI/Memory/AgentMemoryDataAssets.h"
#include "STTConstructCharacterPrompt.generated.h"

/**
 * 
 */
UCLASS()
class MASTERARBEIT_API USTTConstructCharacterPrompt : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	USystemPromptDataAsset* SystemPromptDataAsset;

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) override;
};
