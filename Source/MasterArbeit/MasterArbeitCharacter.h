// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "TurnBasedCombatSystem/Public/TBCAbilitySystemComponent.h"
#include "MasterArbeitCharacter.generated.h"

UCLASS(Blueprintable)
class AMasterArbeitCharacter : public ACharacter, public  IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMasterArbeitCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities)
	TObjectPtr<UTBCAbilitySystemComponent> AbilitySystemComponent;

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};

