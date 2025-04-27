// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LandscapeGridInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class ULandscapeGridInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MASTERARBEIT_API ILandscapeGridInterface
{
	GENERATED_BODY()

	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool LocateTileCenterInt(const FVector& CachedDestination, FVector& OutTileCenter);
};
