// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/MasterArbeitCharacter.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

	FCharacterChatMessageTableRow()
	{
	}

	FCharacterChatMessageTableRow(const FCharacterChatMessage& NewCharacterChatMessage);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat")
	FCharacterChatMessage CharacterChatMessage;
};

// ------------------------------------ BEGIN ----------------------------------------------------------------------
// --------------- [OLD] Adaptation for the List / Tree Views :=> UObject <- per -> UWidget architecture -----------
// --------------------------- Now used as simple wrappers ---------------------------------------------------------
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

//Separation of concerns for now
UCLASS()
class UCPTVEW_CharacterSpeech : public UCPTVE_Wrapper
{
	GENERATED_BODY()

	UCPTVEW_CharacterSpeech()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | ChatPanelTreeViewEntry")
	FCharacterChatMessage CharacterChatMessageData;
};
// --------------- Adaptation for the List / Tree Views :=> UObject <- per -> UWidget architecture -----------------
// ------------------------------------ END ------------------------------------------------------------------------

//Separation of concerns for now
UCLASS(Blueprintable, Category="TextCommunication | AbilityPayload")
class UCharacterChatMessageWrapper : public UObject
{
	GENERATED_BODY()

	UCharacterChatMessageWrapper()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | AbilityPayload")
	FCharacterChatMessage CharacterChatMessage;

	UFUNCTION(BlueprintCallable, Category="TextCommunication | AbilityPayload")
	static UCharacterChatMessageWrapper* CreateWrapper(UObject* Outer, const FCharacterChatMessage& Message);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCharacterChatMessage, const FCharacterChatMessage&,
                                            CharacterChatMessageWrapper);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TextCommunication",
		meta=(RequiredAssetDataTags="RowStructure=/Script/MasterArbeit.FCharacterChatMessageTableRow"))
	UDataTable* CharacterChatMessages;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="TextCommunication")
	void AddCharacterChatMessage(FCharacterChatMessage CharacterChatMessage);
};
