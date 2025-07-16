// Fill out your copyright notice in the Description page of Project Settings.


#include "GMInGame.h"

#include "GameFramework/HUD.h"

AGMInGame::AGMInGame(const FObjectInitializer& ObjectInitializer)
{
	DefaultPawnClass = NULL;

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/LongLiveMadness/Player/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/LongLiveMadness/Player/HUD_InGame"));
	if (HUDBPClass.Class != NULL)
	{
		HUDClass = HUDBPClass.Class;
	}
}
