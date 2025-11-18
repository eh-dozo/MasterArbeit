// Fill out your copyright notice in the Description page of Project Settings.

#include "GATSimpleCharacterMoveTo.h"

#include "GameFramework/Character.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Runtime/AIModule/Classes/Navigation/PathFollowingComponent.h"

void UGATSimpleCharacterMoveTo::Activate()
{
	SetWaitingOnAvatar();

	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwnerActor());
	if (!OwningCharacter)
	{
		EndTask();
		return;
	}

	AAIController* OwningController = Cast<AAIController>(OwningCharacter->GetController());
	if (!OwningController)
	{
		EndTask();
		return;
	}

	CachedAIController = OwningController;

	OwningController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
		this,
		&UGATSimpleCharacterMoveTo::OnPathComponentRequestFinishedCallback);

	OwningController->MoveToLocation(TargetLocation, 50, true, true, true);
}

UGATSimpleCharacterMoveTo* UGATSimpleCharacterMoveTo::WaitForCharacterMovementToComplete(
	UGameplayAbility* OwningAbility, const FVector InTargetLocation)
{
	UGATSimpleCharacterMoveTo* MyObj = NewAbilityTask<UGATSimpleCharacterMoveTo>(OwningAbility);
	MyObj->TargetLocation = InTargetLocation;
	MyObj->ReadyForActivation();
	return MyObj;
}

//TODO: register action failure for agent? (e.g.: tripped off)
void UGATSimpleCharacterMoveTo::OnPathComponentRequestFinishedCallback(FAIRequestID RequestID,
                                                                       const FPathFollowingResult& Result)
{
	EndTask();

	if (Result.IsSuccess())
	{
		OnMovementComplete.Broadcast();
	}
	else
	{
		OnMovementFailed.Broadcast();
	}
}

void UGATSimpleCharacterMoveTo::OnDestroy(const bool bInOwnerFinished)
{
	if (CachedAIController)
	{
		CachedAIController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
	}
	Super::OnDestroy(bInOwnerFinished);
}
