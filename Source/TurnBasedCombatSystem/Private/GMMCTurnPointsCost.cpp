// Fill out your copyright notice in the Description page of Project Settings.


#include "GMMCTurnPointsCost.h"

#include "TurnCostGameplayAbility.h"

float UGMMCTurnPointsCost::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const UTurnCostGameplayAbility* Ability = Cast<UTurnCostGameplayAbility>(Spec.GetContext().GetAbility());

	if (Ability == NULL)
	{
		return 0.f;
	}

	return Ability->Cost;
}