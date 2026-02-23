// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MasterArbeit/TextCommunication/TurnActionData.h"
#include "TurnActionTrackerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERARBEIT_API UTurnActionTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurnActionTrackerComponent();

	UFUNCTION(BlueprintCallable, Category = "Turn Tracking")
	void BeginTurn(int32 TurnNumber);

	UFUNCTION(BlueprintCallable, Category = "Turn Tracking")
	void RecordMovement(const FString& Description);

	UFUNCTION(BlueprintCallable, Category = "Turn Tracking")
	void RecordVerbalInteraction(const FString& Interaction);

	// Not used for now -> need to verify papers on what they consider more important to keep or not in this memory type
	UFUNCTION(BlueprintCallable, Category = "Turn Tracking")
	void RecordAbilityUsed(const FString& AbilityName);

	UFUNCTION(BlueprintCallable, Category = "Turn Tracking")
	void SetEnvironmentData(
		const FString& Location,
		const TArray<FString>& NPCsInRange,
		const FString& Terrain,
		const FString& TimeOfDay,
		const FString& Weather
		);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Tracking")
	FTurnActionData GetCurrentTurnData() const { return CurrentTurnData; }

	UFUNCTION(BlueprintCallable, Category = "Turn Tracking")
	FTurnActionData EndTurn();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FTurnActionData CurrentTurnData;

	UPROPERTY()
	bool bIsTurnActive;
};