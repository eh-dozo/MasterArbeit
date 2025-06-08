// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/CombatAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "TBCAbilitySystemComponent.h"

UCombatAttributeSet::UCombatAttributeSet()
{
	/*InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitInitiative(1000.0f);*/
}

void UCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute == GetActionPointsAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxActionPoints());
	}
	if (Attribute == GetMovementPointsAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMovementPoints());
	}

	Super::PreAttributeChange(Attribute, NewValue);
}

void UCombatAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		OnHealthChanged.Broadcast(this, OldValue, NewValue);
	}
	if (Attribute == GetMaxHealthAttribute())
	{
		if (GetHealth() > NewValue)
		{
			UTBCAbilitySystemComponent* TBCASC = static_cast<UTBCAbilitySystemComponent*>(
				GetOwningAbilitySystemComponent());
			check(TBCASC);

			TBCASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
		const float CurrentHealth = GetHealth();
		OnHealthChanged.Broadcast(this, CurrentHealth, CurrentHealth);
	}
	if (Attribute == GetActionPointsAttribute())
	{
		OnActionPointsChanged.Broadcast(this, OldValue, NewValue);
	}
	if (Attribute == GetMaxActionPointsAttribute())
	{
		if (GetActionPoints() > NewValue)
		{
			UTBCAbilitySystemComponent* TBCASC = static_cast<UTBCAbilitySystemComponent*>(
				GetOwningAbilitySystemComponent());
			check(TBCASC);

			TBCASC->ApplyModToAttribute(GetActionPointsAttribute(), EGameplayModOp::Override, NewValue);
		}
		const float CurrentActionPoints = GetActionPoints();
		OnActionPointsChanged.Broadcast(this, CurrentActionPoints, CurrentActionPoints);
	}
	if (Attribute == GetMovementPointsAttribute())
	{
		OnMovementPointsChanged.Broadcast(this, OldValue, NewValue);
	}
	if (Attribute == GetMaxMovementPointsAttribute())
	{
		if (GetMovementPoints() > NewValue)
		{
			UTBCAbilitySystemComponent* TBCASC = static_cast<UTBCAbilitySystemComponent*>(
				GetOwningAbilitySystemComponent());
			check(TBCASC);

			TBCASC->ApplyModToAttribute(GetMovementPointsAttribute(), EGameplayModOp::Override, NewValue);
		}
		const float CurrentMovementPoints = GetMovementPoints();
		OnMovementPointsChanged.Broadcast(this, CurrentMovementPoints, CurrentMovementPoints);
	}
}

void UCombatAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float OldHealthValue = GetHealth();
		const float NewHealthValue = FMath::Clamp(OldHealthValue - GetDamage(), 0.0f, GetMaxHealth());

		if (OldHealthValue != NewHealthValue)
		{
			SetHealth(NewHealthValue);
		}

		SetDamage(0.0f);
	}
	if (Data.EvaluatedData.Attribute == GetHealAttribute())
	{
		const float OldHealthValue = GetHealth();
		const float NewHealthValue = FMath::Clamp(OldHealthValue + GetHeal(), 0.0f, GetMaxHealth());

		SetHealth(NewHealthValue);

		SetHeal(0.0f);
	}
}
