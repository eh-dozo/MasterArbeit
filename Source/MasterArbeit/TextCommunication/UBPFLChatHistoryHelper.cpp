// Fill out your copyright notice in the Description page of Project Settings.

#include "UBPFLChatHistoryHelper.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"

DEFINE_LOG_CATEGORY_STATIC(LogChatHistoryHelper, Log, All);

bool UUBPFLChatHistoryHelper::AddRowToChatHistory(UDataTable* ChatHistoryTable, EChatRole Role, const FString& Content,
                                                  bool bMarkDirty)
{
	if (!ChatHistoryTable)
	{
		UE_LOG(LogChatHistoryHelper, Error, TEXT("AddRowToChatHistory: ChatHistoryTable is null"));
		return false;
	}

	if (ChatHistoryTable->GetRowStruct() != FChatMessage::StaticStruct())
	{
		UE_LOG(LogChatHistoryHelper, Error, TEXT("AddRowToChatHistory: DataTable is not of type FChatMessage"));
		return false;
	}

	FChatMessage NewMessage;
	NewMessage.ChatRole = Role;
	NewMessage.Content = Content;

	const FString RowName = FString::Printf(TEXT("Message_%lld_%d"),
	                                        FDateTime::Now().GetTicks(),
	                                        FMath::Rand());

	ChatHistoryTable->AddRow(FName(*RowName), NewMessage);

	if (bMarkDirty)
	{
		ChatHistoryTable->MarkPackageDirty();
	}

	UE_LOG(LogChatHistoryHelper, Log, TEXT("AddRowToChatHistory: Added message with role %d to table (MarkDirty: %s)"),
	       static_cast<int32>(Role), bMarkDirty ? TEXT("true") : TEXT("false"));
	return true;
}

FString UUBPFLChatHistoryHelper::FormatTurnSummary(
	const FTurnActionData& CurrentTurn,
	const TArray<FTurnActionData>& OtherCharactersTurns,
	const FTurnActionData& OwnPreviousTurn)
{
	FString Summary;

	Summary += TEXT("[TURN SUMMARY]\n");
	Summary += FString::Printf(TEXT("  Turn #: %02d\n"), CurrentTurn.TurnNumber);
	Summary += FString::Printf(TEXT("  Location: %s\n\n"), *CurrentTurn.Location);

	for (const FTurnActionData& OtherTurn : OtherCharactersTurns)
	{
		FString CharacterName = GetCharacterNameString(OtherTurn.ActingCharacter);
		Summary += FString::Printf(TEXT("  %s ACTIONS THIS TURN:\n"), *CharacterName.ToUpper());
		Summary += FString::Printf(TEXT("  - Movement: %s\n"), *FormatMovementAction(OtherTurn.MovementDescription));
		Summary += FString::Printf(
			TEXT("  - Verbal interaction: %s\n\n"), *FormatVerbalAction(OtherTurn.VerbalInteraction));
	}

	if (OwnPreviousTurn.TurnNumber > 0)
	{
		Summary += TEXT("  YOUR ACTIONS FROM LAST TURN (for context):\n");
		Summary += FString::Printf(
			TEXT("  - Movement: you %s\n"), *FormatMovementAction(OwnPreviousTurn.MovementDescription));
		Summary += FString::Printf(
			TEXT("  - Verbal interaction: %s\n\n"), *FormatVerbalAction(OwnPreviousTurn.VerbalInteraction));
	}

	Summary += TEXT("  ENVIRONMENT (Observable Facts):\n");
	Summary += FString::Printf(TEXT("  - Terrain: %s\n"), *CurrentTurn.Terrain);

	if (CurrentTurn.NPCsInRange.Num() > 0)
	{
		Summary += TEXT("  - NPCs in Range: ");
		for (int32 i = 0; i < CurrentTurn.NPCsInRange.Num(); ++i)
		{
			Summary += CurrentTurn.NPCsInRange[i];
			if (i < CurrentTurn.NPCsInRange.Num() - 1)
			{
				Summary += TEXT(", ");
			}
		}
		Summary += TEXT("\n");
	}

	Summary += FString::Printf(TEXT("  - Time: %s\n"), *CurrentTurn.TimeOfDay);
	Summary += FString::Printf(TEXT("  - Weather: %s\n\n"), *CurrentTurn.Weather);

	Summary += TEXT("  AVAILABLE ACTIONS THIS TURN:\n");
	Summary += TEXT("  - Move relative to: ");

	for (int32 i = 0; i < CurrentTurn.NPCsInRange.Num(); ++i)
	{
		Summary += CurrentTurn.NPCsInRange[i];
		if (i < CurrentTurn.NPCsInRange.Num() - 1)
		{
			Summary += TEXT(", ");
		}
	}
	Summary += TEXT("\n");
	Summary += TEXT("  - Speech: Respond verbally or remain silent\n");

	return Summary;
}

FString UUBPFLChatHistoryHelper::GetAvailableActionsString(AMasterArbeitCharacter* Character)
{
	if (!Character)
	{
		return TEXT("No actions available (character is null)");
	}

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Character);
	if (!ASI)
	{
		return TEXT("No actions available (no ability system)");
	}

	const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return TEXT("No actions available (ability system component is null)");
	}

	FString ActionsString = TEXT("Available Actions:\n");

	TArray<FGameplayAbilitySpec*> ActivatableAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(), ActivatableAbilities);

	if (ActivatableAbilities.Num() == 0)
	{
		ActionsString += TEXT("  - No abilities available\n");
		return ActionsString;
	}

	for (const FGameplayAbilitySpec* AbilitySpec : ActivatableAbilities)
	{
		if (AbilitySpec && AbilitySpec->Ability)
		{
			FString AbilityName = AbilitySpec->Ability->GetName();
			AbilityName.RemoveFromStart(TEXT("GA_"));
			AbilityName.RemoveFromEnd(TEXT("_C"));

			ActionsString += FString::Printf(TEXT("  - %s\n"), *AbilityName);
		}
	}

	return ActionsString;
}

FString UUBPFLChatHistoryHelper::GetCharacterNameString(ECharacterGroupName CharacterName)
{
	switch (CharacterName)
	{
	case Player: return TEXT("Mercenary");
	case Green: return TEXT("Aldric");
	case Red: return TEXT("Grimnar");
	case Purple: return TEXT("Lucien");
	case Default:
	default: return TEXT("Unknown");
	}
}

FString UUBPFLChatHistoryHelper::FormatMovementAction(const FString& MovementDescription)
{
	if (MovementDescription.IsEmpty())
	{
		return TEXT("remained in place");
	}
	return MovementDescription;
}

FString UUBPFLChatHistoryHelper::FormatVerbalAction(const FString& VerbalInteraction)
{
	if (VerbalInteraction.IsEmpty())
	{
		return TEXT("remained silent");
	}
	return VerbalInteraction;
}

bool UUBPFLChatHistoryHelper::AddAssistantResponseToHistory(UDataTable* ChatHistoryTable,
                                                            const FString& RawJsonResponse)
{
	if (!ChatHistoryTable)
	{
		UE_LOG(LogChatHistoryHelper, Error, TEXT("AddAssistantResponseToHistory: ChatHistoryTable is null"));
		return false;
	}

	if (RawJsonResponse.IsEmpty())
	{
		UE_LOG(LogChatHistoryHelper, Warning, TEXT("AddAssistantResponseToHistory: RawJsonResponse is empty"));
		return false;
	}

	const bool bSuccess = AddRowToChatHistory(ChatHistoryTable, EChatRole::Assistant, RawJsonResponse);

	if (bSuccess)
	{
		UE_LOG(LogChatHistoryHelper, Log,
		       TEXT("AddAssistantResponseToHistory: Successfully added Assistant message to chat history"));
	}
	else
	{
		UE_LOG(LogChatHistoryHelper, Error,
		       TEXT("AddAssistantResponseToHistory: Failed to add Assistant message to chat history"));
	}

	return bSuccess;
}

bool UUBPFLChatHistoryHelper::ProcessRoundEndForCharacter(
	UDataTable* ChatHistoryTable,
	const FTurnActionData& CharacterTurnData,
	const TArray<FTurnActionData>& OtherCharactersTurns,
	const FTurnActionData& PreviousTurnData)
{
	if (!ChatHistoryTable)
	{
		UE_LOG(LogChatHistoryHelper, Error, TEXT("ProcessRoundEndForCharacter: ChatHistoryTable is null"));
		return false;
	}

	const FString TurnSummary = FormatTurnSummary(CharacterTurnData, OtherCharactersTurns, PreviousTurnData);

	if (TurnSummary.IsEmpty())
	{
		UE_LOG(LogChatHistoryHelper, Warning, TEXT("ProcessRoundEndForCharacter: Generated turn summary is empty"));
		return false;
	}

	const bool bSuccess = AddRowToChatHistory(ChatHistoryTable, EChatRole::User, TurnSummary);

	if (bSuccess)
	{
		UE_LOG(LogChatHistoryHelper, Log,
		       TEXT("ProcessRoundEndForCharacter: Successfully added turn summary for Turn %d"),
		       CharacterTurnData.TurnNumber);
	}
	else
	{
		UE_LOG(LogChatHistoryHelper, Error, TEXT("ProcessRoundEndForCharacter: Failed to add turn summary"));
	}

	return bSuccess;
}

bool UUBPFLChatHistoryHelper::ClearChatHistoryKeepFewShots(UDataTable* ChatHistoryTable, const int32 NumRowsToKeep, const FString& FewShotsRowNamePrefix,
                                                           const bool bMarkDirty)
{
	if (!ChatHistoryTable)
	{
		UE_LOG(LogChatHistoryHelper, Error, TEXT("ClearChatHistoryKeepFewShots: ChatHistoryTable is null"));
		return false;
	}

	if (ChatHistoryTable->GetRowStruct() != FChatMessage::StaticStruct())
	{
		UE_LOG(LogChatHistoryHelper, Error,
		       TEXT("ClearChatHistoryKeepFewShots: DataTable is not of type FChatMessage"));
		return false;
	}

	TArray<FName> AllRowNames = ChatHistoryTable->GetRowNames();

	if (AllRowNames.Num() <= NumRowsToKeep)
	{
		UE_LOG(LogChatHistoryHelper, Log,
		       TEXT("ClearChatHistoryKeepFewShots: Table has %d rows, keeping all (requested to keep %d)"),
		       AllRowNames.Num(), NumRowsToKeep);
		return true;
	}

	TArray<FName> RowsToRemove;
	for (const FName& RowName : AllRowNames)
	{
		if (FString RowNameStr = RowName.ToString();
			!RowNameStr.StartsWith(FewShotsRowNamePrefix))
		{
			RowsToRemove.Add(RowName);
		}
	}

	int32 NumRowsRemoved = 0;
	for (const FName& RowName : RowsToRemove)
	{
		ChatHistoryTable->RemoveRow(RowName);
		NumRowsRemoved++;
	}

	if (bMarkDirty && NumRowsRemoved > 0)
	{
		ChatHistoryTable->MarkPackageDirty();
	}

	UE_LOG(LogChatHistoryHelper, Log,
	       TEXT("ClearChatHistoryKeepFewShots: Removed %d runtime message rows, kept %d rows (MarkDirty: %s)"),
	       NumRowsRemoved, AllRowNames.Num() - NumRowsRemoved, bMarkDirty ? TEXT("true") : TEXT("false"));

	return true;
}
