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
	Super::BeginPlay();
}

void AMasterArbeitPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InGameMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
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

void AMasterArbeitPlayerController::OnSkipScreenReleased()
{
	IInGameHUDInterface::Execute_SkipSkippableScreens(Cast<UObject>(GetHUD()));
}