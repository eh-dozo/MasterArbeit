// Fill out your copyright notice in the Description page of Project Settings.


#include "TextCommunicationSubsystem.h"

UCharacterChatMessageWrapper::UCharacterChatMessageWrapper()
{
	
}

void UTextCommunicationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterChatMessages = NewObject<UDataTable>(this);
	CharacterChatMessages->RowStruct = FCharacterChatMessage::StaticStruct();

	
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

	CharacterChatMessages->AddRow(FName(*RowName), CharacterChatMessage);

	OnNewCharacterChatMessage.Broadcast(CharacterChatMessage);
}
