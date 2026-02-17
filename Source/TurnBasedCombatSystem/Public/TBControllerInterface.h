// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TBComponent.h"
#include "UObject/Interface.h"
#include "TBControllerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, BlueprintType)
class UTBControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TURNBASEDCOMBATSYSTEM_API ITBControllerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FTransform GetOwningActorTransform();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UTBComponent* GetCombatComponent();
};