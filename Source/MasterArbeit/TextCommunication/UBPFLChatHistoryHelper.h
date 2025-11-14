// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TurnActionData.h"
#include "Engine/DataTable.h"
#include "LlamaRunner/Public/LlamaCppSubsystem.h"
#include "UBPFLChatHistoryHelper.generated.h"

UCLASS()
class MASTERARBEIT_API UUBPFLChatHistoryHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Chat History")
	static bool AddRowToChatHistory(UDataTable* ChatHistoryTable, EChatRole Role, const FString& Content,
	                                bool bMarkDirty = false);

	UFUNCTION(BlueprintCallable, Category = "Chat History")
	static FString FormatTurnSummary(
		const FTurnActionData& CurrentTurn,
		const TArray<FTurnActionData>& OtherCharactersTurns,
		const FTurnActionData& OwnPreviousTurn
	);

	// Not used for now -> static insertion of available actions in FormatTurnSummary
	//					-> (not enough actions implemented)
	UFUNCTION(BlueprintCallable, Category = "Chat History")
	static FString GetAvailableActionsString(class AMasterArbeitCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Chat History")
	static bool AddAssistantResponseToHistory(UDataTable* ChatHistoryTable, const FString& RawJsonResponse);

	UFUNCTION(BlueprintCallable, Category = "Chat History")
	static bool ProcessRoundEndForCharacter(
		UDataTable* ChatHistoryTable,
		const FTurnActionData& CharacterTurnData,
		const TArray<FTurnActionData>& OtherCharactersTurns,
		const FTurnActionData& PreviousTurnData
	);

	UFUNCTION(BlueprintCallable, Category = "Chat History", meta = (AdvancedDisplay = "NumRowsToKeep, bMarkDirty"))
	static bool ClearChatHistoryKeepFewShots(UDataTable* ChatHistoryTable,
	                                         const int32 NumRowsToKeep = 3,
	                                         const FString& FewShotsRowNamePrefix = FString("FewShot_"),
	                                         const bool bMarkDirty = false);

private:
	static FString GetCharacterNameString(ECharacterGroupName CharacterName);

	static FString FormatMovementAction(const FString& MovementDescription);

	static FString FormatVerbalAction(const FString& VerbalInteraction);
};
