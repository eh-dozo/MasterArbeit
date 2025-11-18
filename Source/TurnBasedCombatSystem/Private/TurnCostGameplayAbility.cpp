// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnCostGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UTurnCostGameplayAbility::UTurnCostGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  Cost(0),
	  EffectiveCost(0)
{
}

void UTurnCostGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	// [ARCHIVE] -> BP replicate in GA_PlayerMovement
	// Used for e.g. player movement with dynamic distance range given per magnitude
	/*
		EffectiveCost = TriggerEventData->EventMagnitude;

		if (const FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(
				CostGameplayEffectClass, GetAbilityLevel());
			EffectSpecHandle.IsValid())
		{
			if (const FGameplayTag TurnCostTag = FGameplayTag::RequestGameplayTag(
					TEXT("Abilities.Data.TurnCost"), false);
				TurnCostTag.IsValid())
			{
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, TurnCostTag,
				                                                              EffectiveCost);
				// Cache the spec for use in CheckCost and ApplyCost instead of applying directly
				CachedCostEffectSpec = EffectSpecHandle;
			}
		}
	*/


	EffectiveCost = Cost; // fallback to default set in GA_*

	// Only after dynamic cost calculation and cost commit check passed
	// -> run the blueprint script
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UTurnCostGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          bool bReplicateEndAbility,
                                          bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	OnAbilityEnded.Broadcast();
}

bool UTurnCostGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (CachedCostEffectSpec.IsValid())
	{
		const FGameplayEffectSpec* Spec = CachedCostEffectSpec.Data.Get();
		if (Spec && Spec->Def)
		{
			bool bUsesSetByCaller = false;
			for (const FGameplayModifierInfo& ModInfo : Spec->Def->Modifiers)
			{
				if (ModInfo.ModifierMagnitude.GetMagnitudeCalculationType() ==
					EGameplayEffectMagnitudeCalculation::SetByCaller)
				{
					bUsesSetByCaller = true;
					break;
				}
			}

			if (bUsesSetByCaller)
			{
				UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
				if (ensure(ASC))
				{
					// Copy of FActiveGameplayEffectsContainer::CanApplyAttributeModifiers
					FGameplayEffectSpec TempSpec = *Spec;
					TempSpec.CalculateModifierMagnitudes();

					for (int32 ModIdx = 0; ModIdx < TempSpec.Modifiers.Num(); ++ModIdx)
					{
						const FGameplayModifierInfo& ModDef = TempSpec.Def->Modifiers[ModIdx];
						const FModifierSpec& ModSpec = TempSpec.Modifiers[ModIdx];

						if (ModDef.ModifierOp == EGameplayModOp::Additive)
						{
							if (!ModDef.Attribute.IsValid())
							{
								continue;
							}
							float CurrentValue = ASC->GetNumericAttribute(ModDef.Attribute);
							float CostValue = ModSpec.GetEvaluatedMagnitude();

							if (CurrentValue + CostValue < 0.f)
							{
								const FGameplayTag& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;
								if (OptionalRelevantTags && CostTag.IsValid())
								{
									OptionalRelevantTags->AddTag(CostTag);
								}
								return false;
							}
						}
					}
					return true;
				}
			}
		}
	}

	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UTurnCostGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CachedCostEffectSpec.IsValid())
	{
		UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
		if (ASC)
		{
			ASC->ApplyGameplayEffectSpecToSelf(*CachedCostEffectSpec.Data.Get());
			CachedCostEffectSpec = FGameplayEffectSpecHandle();
		}
	}
	else
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
	}
}
