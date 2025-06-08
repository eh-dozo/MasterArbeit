// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/MasterArbeitCharacter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TextCommunicationSubsystem.generated.h"


USTRUCT(BlueprintType, Category="TextCommunication | UIChat")
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

UCLASS(Blueprintable, Category="TextCommunication | AbilityPayload")
class UCharacterChatMessageWrapper : public UObject
{
	GENERATED_BODY()

	UCharacterChatMessageWrapper()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true, EditCondition="!bIsDelimiter"),
		Category="TextCommunication | AbilityPayload")
	FCharacterChatMessage CharacterChatMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true),
		Category="TextCommunication | AbilityPayload")
	bool bIsDelimiter = false;

	UFUNCTION(BlueprintCallable, Category="TextCommunication | AbilityPayload")
	static UCharacterChatMessageWrapper* CreateWrapper(UObject* Outer, const FCharacterChatMessage& Message);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCharacterChatMessage, const UCharacterChatMessageWrapper*,
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
