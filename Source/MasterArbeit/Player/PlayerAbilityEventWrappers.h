// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlayerAbilityEventWrappers.generated.h"

USTRUCT(BlueprintType, Category="Player | AbilityPayload")
struct FPlayerAbilityMovementPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation;
};

/**
 * 
 */
UCLASS(Blueprintable, Category="Player | AbilityPayload")
class MASTERARBEIT_API UPlayerAbilityMovementPayloadWrapper : public UObject
{
	GENERATED_BODY()

	UPlayerAbilityMovementPayloadWrapper() {}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="Player | AbilityPayload")
	FPlayerAbilityMovementPayload PlayerAbilityMovementPayload;
};