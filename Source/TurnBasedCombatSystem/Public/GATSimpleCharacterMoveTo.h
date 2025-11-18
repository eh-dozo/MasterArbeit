// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "GATSimpleCharacterMoveTo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitSimpleCharacterMoveToDelegate);

/**
 * The below listed task's functions are ordered as: input(Vector) -> (exec) -> outputs(void)
 */
UCLASS()
class TURNBASEDCOMBATSYSTEM_API UGATSimpleCharacterMoveTo : public UAbilityTask
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable,
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"),
		Category = "Ability|Tasks")
	static UGATSimpleCharacterMoveTo* WaitForCharacterMovementToComplete(UGameplayAbility* OwningAbility,
	                                                                     FVector InTargetLocation);

	virtual void Activate() override;

	void OnPathComponentRequestFinishedCallback(FAIRequestID RequestID, const FPathFollowingResult& Result);

	UPROPERTY(BlueprintAssignable)
	FWaitSimpleCharacterMoveToDelegate OnMovementComplete;

	UPROPERTY(BlueprintAssignable)
	FWaitSimpleCharacterMoveToDelegate OnMovementFailed;

private:
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController;
};
