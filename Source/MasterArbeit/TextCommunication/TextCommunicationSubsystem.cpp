// Fill out your copyright notice in the Description page of Project Settings.


#include "TextCommunicationSubsystem.h"

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
}

void UTextCommunicationSubsystem::AddCharacterChatMessage(FCharacterChatMessage CharacterChatMessage)
{
	if (!IsValid(CharacterChatMessages))
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterChatMessages DataTable is invalid"));
		return;
	}

	if (CharacterChatMessage.Message.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add empty message"));
		return;
	}

	FString RowName = FString::Printf(TEXT("%s_%lld"),
	                                  *UEnum::GetValueAsString(CharacterChatMessage.CharacterName.GetValue()),
	                                  FDateTime::Now().GetTicks());
	FCharacterChatMessageTableRow NewCCMTableRow = FCharacterChatMessageTableRow(CharacterChatMessage);

	CharacterChatMessages->AddRow(FName(*RowName), NewCCMTableRow);
	FCharacterChatMessage* NewCharacterChatMessage =
		&CharacterChatMessages->FindRow<FCharacterChatMessageTableRow>(
			                     FName(*RowName),
			                     "CharacterChatMessages::GetNewlyAddedRow",
			                     true)
		                     ->CharacterChatMessage;

	if (NewCharacterChatMessage != NULL)
	{
		OnNewCharacterChatMessage.Broadcast(UCharacterChatMessageWrapper::CreateWrapper(this, *NewCharacterChatMessage));
	}
}
