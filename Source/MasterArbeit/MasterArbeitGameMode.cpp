// Copyright Epic Games, Inc. All Rights Reserved.

#include "MasterArbeitGameMode.h"
#include "MasterArbeitPlayerController.h"
#include "MasterArbeitCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"

AMasterArbeitGameMode::AMasterArbeitGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AMasterArbeitPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/LongLiveMadness/Blueprint/Core/Player/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/LongLiveMadness/Blueprint/Core/Player/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/LongLiveMadness/Blueprint/Core/HUD_InGame"));
	if (HUDBPClass.Class != NULL)
	{
		HUDClass = HUDBPClass.Class;
	}
}
