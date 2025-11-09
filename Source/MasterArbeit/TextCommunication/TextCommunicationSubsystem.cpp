// Fill out your copyright notice in the Description page of Project Settings.

#include "TextCommunicationSubsystem.h"
#include "LlamaCppSubsystem.h"
#include "ModelResponseParser.h"
#include "Dom/JsonObject.h"
#include "MasterArbeit/AI/Memory/AgentMemoryDataAssets.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogTextCommunicationSubsystem, Log, All);

FCharacterChatMessageTableRow::FCharacterChatMessageTableRow(const FCharacterChatMessage& NewCharacterChatMessage)
{
	CharacterChatMessage = NewCharacterChatMessage;
}

UCharacterChatMessageWrapper* UCharacterChatMessageWrapper::CreateWrapper(UObject* Outer,
                                                                          const FCharacterChatMessage& Message)
{
	UCharacterChatMessageWrapper* Wrapper = NewObject<UCharacterChatMessageWrapper>(Outer);
	Wrapper->CharacterChatMessage = Message;
	return Wrapper;
}

void UTextCommunicationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterChatMessages = NewObject<UDataTable>(this);
	CharacterChatMessages->RowStruct = FCharacterChatMessageTableRow::StaticStruct();

	if (const UDSLTextCommunicationSubsystemSettings* Settings = GetDefault<UDSLTextCommunicationSubsystemSettings>();
		Settings && Settings->SystemPromptsToExtract.Num() > 0)
	{
		for (const TSoftObjectPtr<USystemPromptDataAsset>& SystemPromptPtr : Settings->SystemPromptsToExtract)
		{
			if (const USystemPromptDataAsset* SystemPrompt = SystemPromptPtr.LoadSynchronous();
				IsValid(SystemPrompt))
			{
				ExtractFewShotDialogsFromSystemPrompt(SystemPrompt, Settings->AldricCharacterName,
				                                      Settings->MercenaryCharacterName);
			}
			else
			{
				UE_LOG(LogTextCommunicationSubsystem, Warning, TEXT("Failed to load SystemPromptDataAsset: %s"),
				       *SystemPromptPtr.ToString());
			}
		}
	}
	else
	{
		UE_LOG(LogTextCommunicationSubsystem, Display,
		       TEXT("No SystemPromptDataAssets configured in Project Settings > Project > TextCommunication Subsystem"
		       ));
	}
}

void UTextCommunicationSubsystem::AddCharacterChatMessage(FCharacterChatMessage CharacterChatMessage, bool bBroadcast)
{
	if (!IsValid(CharacterChatMessages))
	{
		UE_LOG(LogTextCommunicationSubsystem, Error, TEXT("CharacterChatMessages DataTable is invalid"));
		return;
	}

	if (CharacterChatMessage.Message.IsEmpty())
	{
		UE_LOG(LogTextCommunicationSubsystem, Warning, TEXT("Cannot add empty message"));
		return;
	}

	const FString RowName = FString::Printf(TEXT("%s_%lld"),
	                                        *UEnum::GetValueAsString(CharacterChatMessage.CharacterName.GetValue()),
	                                        FDateTime::Now().GetTicks());
	const FCharacterChatMessageTableRow NewCCMTableRow = FCharacterChatMessageTableRow(CharacterChatMessage);

	CharacterChatMessages->AddRow(FName(*RowName), NewCCMTableRow);

	if (bBroadcast)
	{
		const FCharacterChatMessage* NewCharacterChatMessage =
			&CharacterChatMessages->FindRow<FCharacterChatMessageTableRow>(
				                      FName(*RowName),
				                      "CharacterChatMessages::GetNewlyAddedRow",
				                      true)
			                      ->CharacterChatMessage;
		if (NewCharacterChatMessage != nullptr)
		{
			OnNewCharacterChatMessage.Broadcast(*NewCharacterChatMessage);
		}
	}
}

FString UTextCommunicationSubsystem::ExtractDialogFromAssistantMessage(const FString& JsonContent)
{
	FJsonObjectWrapper JsonWrapper;

	if (FString ErrorMessage;
		!UModelResponseParser::ParseJsonFromString(JsonContent, JsonWrapper, ErrorMessage))
	{
		UE_LOG(LogTextCommunicationSubsystem, Warning, TEXT("Failed to parse Assistant message JSON: %s"),
		       *ErrorMessage);
		return FString();
	}

	if (!JsonWrapper.JsonObject.IsValid())
	{
		return FString();
	}

	const TSharedPtr<FJsonObject>* VerbalInteractionObject;
	if (!JsonWrapper.JsonObject->TryGetObjectField(TEXT("verbal-interaction"), VerbalInteractionObject))
	{
		UE_LOG(LogTextCommunicationSubsystem, Warning, TEXT("verbal-interaction field not found in Assistant message"));
		return FString();
	}

	FString Content;
	if (!(*VerbalInteractionObject)->TryGetStringField(TEXT("content"), Content))
	{
		UE_LOG(LogTextCommunicationSubsystem, Warning, TEXT("content field not found in verbal-interaction"));
		return FString();
	}

	if (FString ResponseType;
		(*VerbalInteractionObject)->TryGetStringField(TEXT("response-type"), ResponseType))
	{
		if (ResponseType.ToLower() == TEXT("silence"))
		{
			return FString();
		}
	}

	return Content;
}

/**
 *	\n Current search pattern ->: "  - Verbal interaction: said \"...\"" \n\n
 *
 * @param TurnSummaryContent the user prompt row content from a few-shot DataTable (e.g. Aldric_Base_02.uasset)
 */
FString UTextCommunicationSubsystem::ExtractDialogFromUserTurnSummary(const FString& TurnSummaryContent)
{
	TArray<FString> Lines;
	TurnSummaryContent.ParseIntoArrayLines(Lines);

	for (const FString& Line : Lines)
	{
		if (FString TrimmedLine = Line.TrimStartAndEnd();
			TrimmedLine.Contains(TEXT("Verbal interaction:"), ESearchCase::IgnoreCase))
		{
			if (TrimmedLine.Contains(TEXT("remained silent"), ESearchCase::IgnoreCase))
			{
				return FString();
			}

			int32 SaidIndex = TrimmedLine.Find(TEXT("said \""), ESearchCase::IgnoreCase);
			if (SaidIndex != INDEX_NONE)
			{
				int32 OpenQuoteIndex = SaidIndex + 5; // "said " = 5 chars
				int32 CloseQuoteIndex = TrimmedLine.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart,
				                                         OpenQuoteIndex + 1);

				if (CloseQuoteIndex != INDEX_NONE)
				{
					FString Dialog = TrimmedLine.Mid(OpenQuoteIndex + 1, CloseQuoteIndex - OpenQuoteIndex - 1);
					return Dialog;
				}
			}
		}
	}

	return FString();
}

bool UTextCommunicationSubsystem::IsDialogAlreadyInChatMessages(const FString& DialogText) const
{
	if (!IsValid(CharacterChatMessages))
	{
		return false;
	}

	TArray<FCharacterChatMessageTableRow*> AllRows;
	CharacterChatMessages->GetAllRows<FCharacterChatMessageTableRow>("", AllRows);

	// TODO: Must be optimized if large strings
	for (const FCharacterChatMessageTableRow* Row : AllRows)
	{
		if (Row && Row->CharacterChatMessage.Message.ToString().Equals(DialogText))
		{
			return true;
		}
	}

	return false;
}

void UTextCommunicationSubsystem::AddDialogToChatMessages(const FString& DialogText,
                                                          TEnumAsByte<ECharacterGroupName> CharacterName,
                                                          const FLinearColor& ThemeColor)
{
	if (DialogText.IsEmpty())
	{
		return;
	}

	FCharacterChatMessage NewMessage;
	NewMessage.CharacterName = CharacterName;
	NewMessage.Message = FText::FromString(DialogText);
	NewMessage.ThemeColor = ThemeColor;

	AddCharacterChatMessage(NewMessage, false);

	// TODO: temp
	UE_LOG(LogTextCommunicationSubsystem, Display, TEXT("Added few-shot dialog to CharacterChatMessages: [%s] %s"),
	       *UEnum::GetValueAsString(CharacterName.GetValue()), *DialogText);
}

void UTextCommunicationSubsystem::ExtractFewShotDialogsFromSystemPrompt(
	const USystemPromptDataAsset* SystemPromptAsset,
	const TEnumAsByte<ECharacterGroupName> CharacterName,
	const TEnumAsByte<ECharacterGroupName> PlayerName)
{
	if (!IsValid(SystemPromptAsset))
	{
		UE_LOG(LogTextCommunicationSubsystem, Warning,
		       TEXT("Invalid SystemPromptDataAsset provided for few-shot extraction"));
		return;
	}

	if (!IsValid(SystemPromptAsset->FewShotChatHistory))
	{
		UE_LOG(LogTextCommunicationSubsystem, Warning,
		       TEXT("No FewShotChatHistory DataTable in SystemPromptDataAsset"));
		return;
	}

	UE_LOG(LogTextCommunicationSubsystem, Display, TEXT("Extracting few-shot dialogs from SystemPromptDataAsset..."));

	TArray<FChatMessage*> FewShotMessages;
	SystemPromptAsset->FewShotChatHistory->GetAllRows<FChatMessage>("", FewShotMessages);

	int32 ExtractedCount = 0;
	int32 SkippedCount = 0;

	for (const FChatMessage* Message : FewShotMessages)
	{
		if (!Message)
		{
			continue;
		}

		FString DialogText;
		TEnumAsByte<ECharacterGroupName> SpeakerName;
		FLinearColor SpeakerColor;

		if (Message->ChatRole == EChatRole::Assistant)
		{
			DialogText = ExtractDialogFromAssistantMessage(Message->Content);
			SpeakerName = CharacterName;
			SpeakerColor = FLinearColor(0.541176f, 0.65098f, 0.137255f);
		}
		else if (Message->ChatRole == EChatRole::User)
		{
			DialogText = ExtractDialogFromUserTurnSummary(Message->Content);
			SpeakerName = PlayerName;
			SpeakerColor = FLinearColor(0.070588f, 0.215686f, 0.592157f);
		}
		else
		{
			continue;
		}

		if (DialogText.IsEmpty())
		{
			SkippedCount++;
			continue;
		}

		if (IsDialogAlreadyInChatMessages(DialogText))
		{
			UE_LOG(LogTextCommunicationSubsystem, Display, TEXT("Skipping duplicate dialog: %s"), *DialogText);
			SkippedCount++;
			continue;
		}

		AddDialogToChatMessages(DialogText, SpeakerName, SpeakerColor);
		ExtractedCount++;
	}

	UE_LOG(LogTextCommunicationSubsystem, Display, TEXT("Few-shot extraction complete. Extracted: %d, Skipped: %d"),
	       ExtractedCount, SkippedCount);
}
