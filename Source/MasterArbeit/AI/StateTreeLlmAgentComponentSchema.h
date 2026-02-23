// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponentSchema.h"
#include "Memory/AgentMemoryDataAssets.h"

#include "StateTreeLlmAgentComponentSchema.generated.h"

class AAIController;

/**
 * (UNUSED): wanted to have a custom StateTree schema which would have allowed to have defaults parameters for agents
 * configs. But it buggy in UE5.5
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories,
	meta = (DisplayName = "StateTree Llm Agent Component", CommonSchema))
class MASTERARBEIT_API UStateTreeLlmAgentComponentSchema : public UStateTreeComponentSchema
{
	GENERATED_BODY()

public:
	UStateTreeLlmAgentComponentSchema(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostLoad() override;

	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;

	static bool SetContextRequirements(UBrainComponent& BrainComponent, FStateTreeExecutionContext& Context,
		bool bLogErrors = false);

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif

protected:
	UPROPERTY(EditAnywhere, Category = "Defaults", NoClear)
	TSubclassOf<AAIController> AIControllerClass = nullptr;

	/**
	 *                           ||||||||||
	 * (UNUSED): Here for e.g. - vvvvvvvvvv
	 */
	/*UPROPERTY(EditAnywhere, Category = "Defaults", NoClear)
	USystemPromptDataAsset* SystemPromptDataAsset = nullptr;*/
};