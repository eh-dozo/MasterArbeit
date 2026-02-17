// Copyright Epic Games, Inc. All Rights Reserved.

#include "MasterArbeitCharacter.h"

#include "AbilitySystemGlobals.h"
#include "GameplayAbilitiesModule.h"
#include "Attributes/CombatAttributeSet.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"

AMasterArbeitCharacter::AMasterArbeitCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = CreateDefaultSubobject<UTBCAbilitySystemComponent>(TEXT("ASC"));
	//CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));
}

void AMasterArbeitCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMasterArbeitCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMasterArbeitCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddSet<UCombatAttributeSet>();

		IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals()
		                               ->GetAttributeSetInitter()
		                               ->InitAttributeSetDefaults(
			                               AbilitySystemComponent,
			                               StaticEnum<ECharacterGroupName>()->GetNameByValue(
				                               CharacterGroupName.GetIntValue()),
			                               1,
			                               true);
	}
}

UAbilitySystemComponent* AMasterArbeitCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}