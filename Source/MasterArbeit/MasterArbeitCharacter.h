// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Attributes/HealthAttributeSet.h"
#include "GameFramework/Character.h"
#include "TBCAbilitySystemComponent.h"
#include "MasterArbeitCharacter.generated.h"

UCLASS(Blueprintable)
class AMasterArbeitCharacter : public ACharacter, public  IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMasterArbeitCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities)
	TObjectPtr<UTBCAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities)
	TObjectPtr<UHealthAttributeSet> HealthSet;

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};

