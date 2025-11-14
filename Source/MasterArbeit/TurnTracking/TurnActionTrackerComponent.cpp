// Fill out your copyright notice in the Description page of Project Settings.

#include "TurnActionTrackerComponent.h"
#include "MasterArbeit/MasterArbeitCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogActionTrackerComponent, Log, All);

UTurnActionTrackerComponent::UTurnActionTrackerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsTurnActive = false;
}

void UTurnActionTrackerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AMasterArbeitCharacter* OwnerCharacter = Cast<AMasterArbeitCharacter>(GetOwner()))
	{
		CurrentTurnData.ActingCharacter = OwnerCharacter->CharacterGroupName;
	}
}

void UTurnActionTrackerComponent::BeginTurn(int32 TurnNumber)
{
	CurrentTurnData = FTurnActionData();
	CurrentTurnData.TurnNumber = TurnNumber;

	if (const AMasterArbeitCharacter* OwnerCharacter = Cast<AMasterArbeitCharacter>(GetOwner()))
	{
		CurrentTurnData.ActingCharacter = OwnerCharacter->CharacterGroupName;
	}

	bIsTurnActive = true;

	UE_LOG(LogActionTrackerComponent, Log, TEXT("TurnActionTracker: Begin Turn %d for %s"),
		TurnNumber,
		*UEnum::GetValueAsString(CurrentTurnData.ActingCharacter.GetValue()));
}

void UTurnActionTrackerComponent::RecordMovement(const FString& Description)
{
	if (!bIsTurnActive)
	{
		UE_LOG(LogActionTrackerComponent, Warning, TEXT("TurnActionTracker: Attempting to record movement while no turn is active"));
		return;
	}

	CurrentTurnData.MovementDescription = Description;
	UE_LOG(LogActionTrackerComponent, Log, TEXT("TurnActionTracker: Recorded movement: %s"), *Description);
}

void UTurnActionTrackerComponent::RecordVerbalInteraction(const FString& Interaction)
{
	if (!bIsTurnActive)
	{
		UE_LOG(LogActionTrackerComponent, Warning, TEXT("TurnActionTracker: Attempting to record verbal interaction while no turn is active"));
		return;
	}

	CurrentTurnData.VerbalInteraction = Interaction;
	UE_LOG(LogActionTrackerComponent, Log, TEXT("TurnActionTracker: Recorded verbal interaction: %s"), *Interaction);
}

void UTurnActionTrackerComponent::RecordAbilityUsed(const FString& AbilityName)
{
	if (!bIsTurnActive)
	{
		UE_LOG(LogActionTrackerComponent, Warning, TEXT("TurnActionTracker: Attempting to record ability while no turn is active"));
		return;
	}

	CurrentTurnData.AbilitiesUsed.AddUnique(AbilityName);
	UE_LOG(LogActionTrackerComponent, Log, TEXT("TurnActionTracker: Recorded ability: %s"), *AbilityName);
}

void UTurnActionTrackerComponent::SetEnvironmentData(
	const FString& Location,
	const TArray<FString>& NPCsInRange,
	const FString& Terrain,
	const FString& TimeOfDay,
	const FString& Weather)
{
	CurrentTurnData.Location = Location;
	CurrentTurnData.NPCsInRange = NPCsInRange;
	CurrentTurnData.Terrain = Terrain;
	CurrentTurnData.TimeOfDay = TimeOfDay;
	CurrentTurnData.Weather = Weather;

	UE_LOG(LogActionTrackerComponent, Log, TEXT("TurnActionTracker: Set environment data - Location: %s, NPCs: %d"),
		*Location, NPCsInRange.Num());
}

FTurnActionData UTurnActionTrackerComponent::EndTurn()
{
	if (!bIsTurnActive)
	{
		UE_LOG(LogActionTrackerComponent, Warning, TEXT("TurnActionTracker: Attempting to end turn while no turn is active"));
		return FTurnActionData();
	}

	bIsTurnActive = false;

	UE_LOG(LogActionTrackerComponent, Log, TEXT("TurnActionTracker: End Turn %d"), CurrentTurnData.TurnNumber);

	return CurrentTurnData;
}
