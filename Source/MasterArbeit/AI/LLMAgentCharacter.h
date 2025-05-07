// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/MasterArbeitCharacter.h"
#include "LLMAgentCharacter.generated.h"

//DEPECRATED: BP class now inherit from MasterArbeitCharacter directly (because of shared editor properties like HealthBar Widget)
UCLASS()
class MASTERARBEIT_API ALLMAgentCharacter : public AMasterArbeitCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALLMAgentCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
