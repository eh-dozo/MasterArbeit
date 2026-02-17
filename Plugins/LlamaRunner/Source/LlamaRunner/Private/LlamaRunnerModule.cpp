// Copyright Epic Games, Inc. All Rights Reserved.

#include "LlamaRunnerModule.h"

//THIRD_PARTY_INCLUDES_START
#include "llama.h"
//THIRD_PARTY_INCLUDES_END

#define LOCTEXT_NAMESPACE "FLlamaRunnerModule"

void FLlamaRunnerModule::StartupModule()
{
	llama_backend_init();
}

void FLlamaRunnerModule::ShutdownModule()
{
	llama_backend_free();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLlamaRunnerModule, LlamaRunner)