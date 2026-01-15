// Copyright (c) 2025 Eliot Hoff - HTW Berlin - Master's Thesis Project

#include "MasterArbeitGameInstance.h"

void UMasterArbeitGameInstance::SetParticipantID(const FString& InParticipantID)
{
	ParticipantID = InParticipantID;
	bHasParticipantID = true;
}
