// Copyright (c) 2025 Eliot Hoff - HTW Berlin - Master's Thesis Project

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MasterArbeitGameInstance.generated.h"

/**
 * Initially created for the participant experience: to store their ID after game launch
 */
UCLASS()
class MASTERARBEIT_API UMasterArbeitGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** Sets the participant ID for this session */
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	void SetParticipantID(const FString& InParticipantID);

	/** Gets the participant ID for this session */
	UFUNCTION(BlueprintPure, Category = "Experiment")
	FString GetParticipantID() const { return ParticipantID; }

	/** Returns true if a participant ID has been set */
	UFUNCTION(BlueprintPure, Category = "Experiment")
	bool HasParticipantID() const { return bHasParticipantID; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Experiment")
	FString ParticipantID;

	UPROPERTY(BlueprintReadOnly, Category = "Experiment")
	bool bHasParticipantID = false;
};
