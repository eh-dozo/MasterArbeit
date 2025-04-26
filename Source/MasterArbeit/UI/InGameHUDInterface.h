// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InGameHUDInterface.generated.h"

UINTERFACE()
class UInGameHUDInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MASTERARBEIT_API IInGameHUDInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SkipSkippableScreens();
};
