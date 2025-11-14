#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/TextCommunication/ModelResponseParser.h"
#include "UObject/Object.h"
#include "ActionPayloads.generated.h"

USTRUCT(BlueprintType, Category="LlamaMovement | ActionPayload")
struct FMovementAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DistanceLiteral = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Target = nullptr;
};

UCLASS(Blueprintable, Category="LlamaMovement | AbilityPayload")
class UMovementActionWrapper : public UObject
{
	GENERATED_BODY()

	UMovementActionWrapper()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="LlamaMovement | AbilityPayload")
	FMovementAction MovementAction;

	UFUNCTION(BlueprintCallable, Category="LlamaMovement | AbilityPayload")
	static UMovementActionWrapper* CreateWrapper(UObject* Outer, const FMovementAction& Action);
};