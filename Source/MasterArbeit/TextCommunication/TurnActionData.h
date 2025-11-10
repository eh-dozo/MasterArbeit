#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/MasterArbeitCharacter.h"
#include "TurnActionData.generated.h"

USTRUCT(BlueprintType, Category="TurnActionData")
struct FTurnActionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TurnNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MovementDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VerbalInteraction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AbilitiesUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> NPCsInRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Terrain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TimeOfDay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Weather;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECharacterGroupName> ActingCharacter;
};
