// Fill out your copyright notice in the Description page of Project Settings.


#include "GMInGame.h"

#include "CharacterTurnStart.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"

/*
 * ===!! WARNING !!===
 * IF one of the default classes BP set below are referencing the BP GM,
 * it will create a loading deadlock, because
 * -> engine is constructing the CDO for AGMInGame
 * -> tries to load the HUD BP
 * -> HUD BP has reference to GM BP which derive from this C++
 * -> results in a loading dependency loop apparently.......
 */

AGMInGame::AGMInGame(const ::FObjectInitializer& ObjectInitializer)
{
	DefaultPawnClass = NULL;

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(
		TEXT("/Game/LongLiveMadness/Player/BP_TopDownPlayerController"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	/*static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/LongLiveMadness/Player/HUD_InGame"));
	if (HUDBPClass.Class != NULL)
	{
		HUDClass = HUDBPClass.Class;
	}*/
}

void AGMInGame::BeginPlay()
{
	Super::BeginPlay();

	//SpawnCharacters();
}

void AGMInGame::SpawnCharacters()
{
	UWorld* World = GetWorld();

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/LongLiveMadness/Player/BP_PlayerCharacter"));
	static ConstructorHelpers::FClassFinder<APawn> GreenPawnBPClass(
		TEXT("/Game/LongLiveMadness/LlamaAgent/Character/BP_Green"));
	static ConstructorHelpers::FClassFinder<APawn> RedPawnBPClass(
		TEXT("/Game/LongLiveMadness/LlamaAgent/Character/BP_Red"));
	static ConstructorHelpers::FClassFinder<APawn> PurplePawnBPClass(
		TEXT("/Game/LongLiveMadness/LlamaAgent/Character/BP_Purple"));

	if (!PlayerPawnBPClass.Class
		|| !GreenPawnBPClass.Class
		|| !RedPawnBPClass.Class
		|| !PurplePawnBPClass.Class)
	{
		UE_LOG(LogGameMode, Error, TEXT("GMInGame_SpawnCharacters: One of the expect 4 BP pawns were not found"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = GetInstigator();
	SpawnParameters.ObjectFlags |= RF_Transient; // We never want to save default player pawns into a map
	for (TActorIterator<ACharacterTurnStart> It(World); It; ++It)
	{
		/*switch (It->CharacterGroupName)
		{
		case Player:
			APawn* PlayerPawn = SpawnCharacterPawnAtCharacterStart(PlayerPawnBPClass, It->GetTransform());
			UGameplayStatics::GetPlayerController(this, 0)->Possess(PlayerPawn);
			break;
		case Green:
			SpawnCharacterPawnAtCharacterStart(GreenPawnBPClass, It->GetTransform());
			break;
		case Red:
			SpawnCharacterPawnAtCharacterStart(RedPawnBPClass, It->GetTransform());
			break;
		case Purple:
			SpawnCharacterPawnAtCharacterStart(PurplePawnBPClass, It->GetTransform());
			break;
		case Default: UE_LOG(LogGameMode, Warning,
		                     TEXT("GMInGame_SpawnCharacters: Default character name found in the starters!"));
		default: UE_LOG(LogGameMode, Warning,
		                TEXT("GMInGame_SpawnCharacters: Default reroute!"));
		}*/
	}
}

APawn* AGMInGame::SpawnCharacterPawnAtCharacterStart(
	const ConstructorHelpers::FClassFinder<APawn>& ClassFinder,
	const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = GetInstigator();
	SpawnParameters.ObjectFlags |= RF_Transient; // We never want to save default player pawns into a map
	APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(ClassFinder.Class, SpawnTransform, SpawnParameters);
	if (!ResultPawn)
	{
		UE_LOG(LogGameMode, Warning,
			TEXT("GMInGame_SpawnCharacterPawnAtCharacterStart: Couldn't spawn Pawn of type %s at %s"),
			*GetNameSafe(ClassFinder.Class), *SpawnTransform.ToHumanReadableString());
	}
	return ResultPawn;
}