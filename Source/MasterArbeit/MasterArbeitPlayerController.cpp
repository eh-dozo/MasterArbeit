// Copyright Epic Games, Inc. All Rights Reserved.

#include "MasterArbeitPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "MasterArbeitCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/HUD.h"
#include "HAL/Platform.h"
#include "Kismet/GameplayStatics.h"
#include "UI/InGameHUDInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMasterArbeitPlayerController::AMasterArbeitPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void AMasterArbeitPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	TArray<AActor*> LandscapeGrids;
	UGameplayStatics::GetAllActorsOfClass(this, ALandscapeGrid::StaticClass(), LandscapeGrids);

	GridActor = Cast<ALandscapeGrid>(LandscapeGrids[0]);
}

void AMasterArbeitPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InGameMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		/*EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this,
		                                   &AMasterArbeitPlayerController::OnMovementInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this,
		                                   &AMasterArbeitPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this,
		                                   &AMasterArbeitPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this,
		                                   &AMasterArbeitPlayerController::OnSetDestinationReleased);*/

		// Setup temporary widget skip event
		EnhancedInputComponent->BindAction(SkipScreenReleaseAction, ETriggerEvent::Completed, this,
		                                   &AMasterArbeitPlayerController::OnSkipScreenReleased);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void AMasterArbeitPlayerController::OnMovementInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down

void AMasterArbeitPlayerController::OnSetDestinationTriggered()
{
	FollowTime += GetWorld()->GetDeltaSeconds();

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit))
	{
		CachedDestination = Hit.Location;
	}

	// Move towards mouse pointer or touch
	/*APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}*/
}

void AMasterArbeitPlayerController::OnSetDestinationReleased()
{
	FVector TileCenterLocation = FVector(0.0f, 0.0f, 0.0f);
	if (GridActor->LocateTileCenter(CachedDestination, TileCenterLocation))
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, TileCenterLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,
		                                               FXCursor,
		                                               TileCenterLocation,
		                                               FRotator::ZeroRotator,
		                                               FVector(1.f, 1.f, 1.f),
		                                               true,
		                                               true,
		                                               ENCPoolMethod::None,
		                                               true);
		UE_LOG(LogTemp, Error, TEXT("%f %f %f"), CachedDestination.X, CachedDestination.Y, CachedDestination.Z);
	}

	FollowTime = 0.f;
}

void AMasterArbeitPlayerController::OnSkipScreenReleased()
{
	IInGameHUDInterface::Execute_SkipSkippableScreens(Cast<UObject>(GetHUD()));
}
