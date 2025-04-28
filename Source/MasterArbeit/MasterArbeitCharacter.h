// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MasterArbeitCharacter.generated.h"

UCLASS(Blueprintable)
class AMasterArbeitCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMasterArbeitCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;
};

