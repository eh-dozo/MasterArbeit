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
	// Used for e.g. player movement with dynamic distance range given per magnitude
	if (TriggerEventData && TriggerEventData->EventMagnitude != 0.f)
	{
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
	}
	else
	{
		EffectiveCost = Cost; // fallback to default set in GA_*
	}

	// Always create and cache spec with SetByCaller for all abilities
	// This prevents errors when GE has SetByCaller modifiers that need to be set


	// CommitAbilityCost now uses our cached spec via overridden CheckCost and ApplyCost
	if (!CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("Not enough for the Commit cost: %s"), *this->GetName());

		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}

	// Only after dynamic cost calculation and cost commit check passed
	// -> run the blueprint script
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
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
			// Check if any modifier uses SetByCaller
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

			// If using SetByCaller, manually check if cost can be afforded
			if (bUsesSetByCaller)
			{
				UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
				if (ensure(ASC))
				{
					// Make a copy and calculate modifier magnitudes to resolve SetByCaller values
					FGameplayEffectSpec TempSpec = *Spec;
					TempSpec.CalculateModifierMagnitudes();

					// Check each modifier (same logic as FActiveGameplayEffectsContainer::CanApplyAttributeModifiers)
					for (int32 ModIdx = 0; ModIdx < TempSpec.Modifiers.Num(); ++ModIdx)
					{
						const FGameplayModifierInfo& ModDef = TempSpec.Def->Modifiers[ModIdx];
						const FModifierSpec& ModSpec = TempSpec.Modifiers[ModIdx];

						// It only makes sense to check additive operators
						if (ModDef.ModifierOp == EGameplayModOp::Additive)
						{
							if (!ModDef.Attribute.IsValid())
							{
								continue;
							}
							// Use public API to get current attribute value
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

	// Fall back to parent implementation if not using SetByCaller or no cached spec
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
			// Apply the cached spec that already has SetByCaller values set
			ASC->ApplyGameplayEffectSpecToSelf(*CachedCostEffectSpec.Data.Get());
			// Clear the cache after use
			CachedCostEffectSpec = FGameplayEffectSpecHandle();
		}
	}
	else
	{
		// Fall back to parent implementation if no cached spec
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
	}
}
