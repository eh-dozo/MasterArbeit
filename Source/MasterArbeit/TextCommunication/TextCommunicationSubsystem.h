// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MasterArbeit/MasterArbeitCharacter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TextCommunicationSubsystem.generated.h"

USTRUCT(BlueprintType, Category="TextCommunication | UIChat")
struct FCharacterChatMessage : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat")
	TEnumAsByte<ECharacterGroupName> CharacterName = Default; //TODO: should be changed to concrete names

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat", meta=(MultiLine))
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | UIChat")
	FLinearColor ThemeColor = FLinearColor::White;
};

UCLASS(Blueprintable, Category="TextCommunication | AbilityPayload")
class UCharacterChatMessageWrapper : public UObject
{
	GENERATED_BODY()

	UCharacterChatMessageWrapper();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TextCommunication | AbilityPayload")
	FCharacterChatMessage CharacterChatMessage;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewCharacterChatMessage, const FCharacterChatMessage&,
                                            CharacterChatMessage);

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
		meta=(RequiredAssetDataTags="RowStructure=/Script/MasterArbeit.FCharacterChatMessage"))
	UDataTable* CharacterChatMessages;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="TextCommunication")
	void AddCharacterChatMessage(FCharacterChatMessage CharacterChatMessage);
};
