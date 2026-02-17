// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/MasterArbeitCharacter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DeveloperSettings.h"
#include "TextCommunicationSubsystem.generated.h"


//CPTVE = ChatPanelTreeView
// + W  =   + Wrapper
// + D  =   + Data

USTRUCT(BlueprintType, Category="TextCommunication | UIChat" /*Also used for CPTVED_CharacterSpeech*/)
struct FCharacterChatMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat")
	TEnumAsByte<ECharacterGroupName> CharacterName = Default; //TODO: should be changed to concrete names

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat", meta=(MultiLine))
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat")
	FLinearColor ThemeColor = FLinearColor::White;
};

USTRUCT(BlueprintType, Category="TextCommunication | UIChat")
struct FCharacterChatMessageTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FCharacterChatMessageTableRow() {}

	FCharacterChatMessageTableRow(const FCharacterChatMessage& NewCharacterChatMessage);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat")
	FCharacterChatMessage CharacterChatMessage;
};


// ------------------------------------ BEGIN ------------------------------------------------------------------------
// --------------------------- simple wrappers USED FOR UI -----------------------------------------------------
USTRUCT(BlueprintType, Category="TextCommunication | ChatPanelTreeViewEntry")
struct FUCPTVED_RoundHeader
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | ChatPanelTreeViewEntry")
	int32 RoundNumber = -1;
};

UCLASS(Abstract, Blueprintable, Category="TextCommunication | ChatPanelTreeViewEntry")
class UCPTVE_Wrapper : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class UCPTVEW_Delimiter : public UCPTVE_Wrapper
{
	GENERATED_BODY()
};

UCLASS()
class UCPTVEW_RoundHeader : public UCPTVE_Wrapper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | ChatPanelTreeViewEntry")
	FUCPTVED_RoundHeader RoundHeaderData;
};


//----------- Separation of concerns for now
UCLASS()
class UCPTVEW_CharacterSpeech : public UCPTVE_Wrapper
{
	GENERATED_BODY()

	UCPTVEW_CharacterSpeech() {}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | ChatPanelTreeViewEntry")
	FCharacterChatMessage CharacterChatMessageData;
};

// ------------------------------------ END ------------------------------------------------------------------------

//Separation of concerns for now
UCLASS(Blueprintable, Category="TextCommunication | AbilityPayload")
class UCharacterChatMessageWrapper : public UObject
{
	GENERATED_BODY()

	UCharacterChatMessageWrapper() {}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | AbilityPayload")
	FCharacterChatMessage CharacterChatMessage;

	UFUNCTION(BlueprintCallable, Category="TextCommunication | AbilityPayload")
	static UCharacterChatMessageWrapper* CreateWrapper(UObject* Outer, const FCharacterChatMessage& Message);
};

UCLASS(Config=Game, ProjectUserConfig, DisplayName="TextCommunication Subsystem Settings")
class UDSLTextCommunicationSubsystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return "Project"; }
	#if WITH_EDITOR
	virtual FText GetSectionText() const override { return FText::FromString("TextCommunication Subsystem"); }

	virtual FText GetSectionDescription() const override
	{
		return FText::FromString(
			"Here to specify which system prompt assets to check to extract pre-written dialogs in the few-shots.");
	}
	#endif

	UPROPERTY(Config, EditAnywhere, Category="Few-Shot Extraction",
		meta=(ToolTip="System Prompt Data Assets whose few-shot chat history will be extracted on game startup"))
	TArray<TSoftObjectPtr<class USystemPromptDataAsset>> SystemPromptsToExtract;

	UPROPERTY(Config, EditAnywhere, Category="Few-Shot Extraction",
		meta=(ToolTip="Character name to use for AI character (Assistant role) in few-shot dialogs"))
	TEnumAsByte<ECharacterGroupName> AldricCharacterName = Green;

	UPROPERTY(Config, EditAnywhere, Category="Few-Shot Extraction",
		meta=(ToolTip="Character name to use for player/mercenary (User role) in few-shot dialogs"))
	TEnumAsByte<ECharacterGroupName> MercenaryCharacterName = Player;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCharacterChatMessage, const FCharacterChatMessage&,
	CharacterChatMessageWrapper);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatMessagesCleared);

/**
 *
 */
UCLASS(Blueprintable, Category="TextCommunication")
class MASTERARBEIT_API UTextCommunicationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="TextCommunication")
	FOnNewCharacterChatMessage OnNewCharacterChatMessage;

	UPROPERTY(BlueprintAssignable, Category="TextCommunication")
	FOnChatMessagesCleared OnChatMessagesCleared;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TextCommunication",
		meta=(RequiredAssetDataTags="RowStructure=/Script/MasterArbeit.FCharacterChatMessageTableRow"))
	UDataTable* CharacterChatMessages;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="TextCommunication")
	void AddCharacterChatMessage(FCharacterChatMessage CharacterChatMessage, bool bBroadcast = true);

	UFUNCTION(BlueprintCallable, Category="TextCommunication")
	void ClearChatMessages(bool bReextractFewShots = true);

private:
	void ExtractFewShotDialogsFromSystemPrompt(const class USystemPromptDataAsset* SystemPromptAsset,
		TEnumAsByte<ECharacterGroupName> CharacterName,
		TEnumAsByte<ECharacterGroupName> PlayerName);
	static FString ExtractDialogFromAssistantMessage(const FString& JsonContent);
	static FString ExtractDialogFromUserTurnSummary(const FString& TurnSummaryContent);

	bool IsDialogAlreadyInChatMessages(const FString& DialogText) const;

	void AddDialogToChatMessages(const FString& DialogText,
		TEnumAsByte<ECharacterGroupName> CharacterName,
		const FLinearColor& ThemeColor);
};