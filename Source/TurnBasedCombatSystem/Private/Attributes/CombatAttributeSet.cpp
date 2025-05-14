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

	Super::PreAttributeChange(Attribute, NewValue);
}

void UCombatAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		OnHealthChanged.Broadcast(this, OldValue, NewValue);
	}
	else if (Attribute == GetMaxHealthAttribute())
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
