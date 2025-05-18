// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TBComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTurnEndDelegate, UTBComponent*, TurnBasedComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTurnStartDelegate);

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TURNBASEDCOMBATSYSTEM_API UTBComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTBComponent();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FTurnEndDelegate OnTurnEnd;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTurnStartDelegate OnTurnStart;

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
