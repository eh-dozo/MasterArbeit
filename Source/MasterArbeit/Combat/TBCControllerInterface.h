// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatComponent.h"
#include "UObject/Interface.h"
#include "TBCControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, BlueprintType)
class UTBCControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MASTERARBEIT_API ITBCControllerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FTransform GetOwningActorTransform();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UCombatComponent* GetCombatComponent();
};
