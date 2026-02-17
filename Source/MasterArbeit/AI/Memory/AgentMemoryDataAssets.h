// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AgentMemoryDataAssets.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class MASTERARBEIT_API USystemPromptDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine))
	FText SystemPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Few-Shot Examples",
		meta=(RequiredAssetDataTags="RowStructure=/Script/LlamaRunner.ChatMessage"))
	UDataTable* FewShotChatHistory;
};