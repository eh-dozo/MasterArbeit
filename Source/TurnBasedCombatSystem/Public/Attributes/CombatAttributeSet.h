// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CombatAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttributeChangedEvent, UAttributeSet*, AttributeSet, float, OldValue, float, NewValue);

/**
 * 
 */
UCLASS()
class TURNBASEDCOMBATSYSTEM_API UCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCombatAttributeSet();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (HideFromModifiers))
	FGameplayAttributeData Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Damage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Heal;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Initiative;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData ActionPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxActionPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MovementPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxMovementPoints;
	
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Heal);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Initiative);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, ActionPoints);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MaxActionPoints);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MovementPoints);
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MaxMovementPoints);

	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnActionPointsChanged;

	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnMovementPointsChanged;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
