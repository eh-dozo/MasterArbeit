// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TurnCostGameplayAbility.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityEnded);

/**
 *
 */
UCLASS()
class TURNBASEDCOMBATSYSTEM_API UTurnCostGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	UTurnCostGameplayAbility(const FObjectInitializer& ObjectInitializer);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Turn Cost Gameplay Ability")
	FOnAbilityEnded OnAbilityEnded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cost;

protected:
	float EffectiveCost;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Cost Gameplay Ability")
	float GetEffectiveCost() const { return EffectiveCost; }

	UFUNCTION(BlueprintCallable, Category = "Turn Cost Gameplay Ability")
	void SetEffectiveCost(const float NewValue) { EffectiveCost = NewValue; }

	UFUNCTION(BlueprintCallable, Category = "Turn Cost Gameplay Ability")
	void SetCachedCostEffectSpec(const FGameplayEffectSpecHandle& SpecHandle) { CachedCostEffectSpec = SpecHandle; }

private:
	mutable FGameplayEffectSpecHandle CachedCostEffectSpec;
};