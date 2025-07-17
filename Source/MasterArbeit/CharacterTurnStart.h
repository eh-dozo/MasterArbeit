// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterArbeitCharacter.h"
#include "Engine/NavigationObjectBase.h"
#include "CharacterTurnStart.generated.h"

UCLASS()
class MASTERARBEIT_API ACharacterTurnStart : public ANavigationObjectBase
{
	GENERATED_BODY()

public:
	ACharacterTurnStart(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = CharacterStart)
	TEnumAsByte<ECharacterGroupName> CharacterGroupName = Default;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = CharacterStart)
	int32 StartTurnIndex = 0;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif
};
