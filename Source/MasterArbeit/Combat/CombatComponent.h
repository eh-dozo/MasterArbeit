// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTurnEndDelegate);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERARBEIT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();

	UPROPERTY(BlueprintAssignable)
	FTurnEndDelegate OnTurnEnd;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Blueprintable, BlueprintCallable, BlueprintImplementableEvent)
	void StartUnitTurn();

	UFUNCTION(Blueprintable, BlueprintCallable, BlueprintImplementableEvent)
	void EndUnitTurn();

	UFUNCTION(Blueprintable, BlueprintCallable, BlueprintImplementableEvent)
	void BeginCombat();
};
