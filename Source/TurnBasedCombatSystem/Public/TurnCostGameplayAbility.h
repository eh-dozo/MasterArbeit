// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TurnCostGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class TURNBASEDCOMBATSYSTEM_API UTurnCostGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	UTurnCostGameplayAbility(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cost;
};
