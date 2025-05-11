// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Attributes/CombatAttributeSet.h"
#include "GameFramework/Character.h"
#include "TBCAbilitySystemComponent.h"
#include "MasterArbeitCharacter.generated.h"

UENUM(BlueprintType)
enum ECharacterGroupName
{
	Default,
	Player,
	Green,
	Red,
	Purple
};

UCLASS(Blueprintable)
class AMasterArbeitCharacter : public ACharacter, public  IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMasterArbeitCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setup)
	TEnumAsByte<ECharacterGroupName> CharacterGroupName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities)
	TObjectPtr<UTBCAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities)
	TObjectPtr<UCombatAttributeSet> CombatAttributeSet;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostInitializeComponents() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};

