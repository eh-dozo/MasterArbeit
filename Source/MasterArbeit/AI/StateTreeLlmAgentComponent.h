// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "StateTreeLlmAgentComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERARBEIT_API UStateTreeLlmAgentComponent : public UStateTreeComponent
{
	GENERATED_BODY()

public:
	//~ BEGIN IStateTreeSchemaProvider
	virtual TSubclassOf<UStateTreeSchema> GetSchema() const override;
	//~ END

	virtual bool SetContextRequirements(FStateTreeExecutionContext& Context, bool bLogErrors = false) override;
};