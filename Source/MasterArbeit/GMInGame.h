// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTurnStart.h"
#include "GameFramework/GameModeBase.h"
#include "GMInGame.generated.h"

/**
* All of the logic was moved to BP
*/
UCLASS()
class MASTERARBEIT_API AGMInGame : public AGameModeBase
{
	GENERATED_BODY()

	AGMInGame(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

private:
	void SpawnCharacters();
	APawn* SpawnCharacterPawnAtCharacterStart(const ConstructorHelpers::FClassFinder<APawn>& ClassFinder,
		const FTransform& SpawnTransform);
};