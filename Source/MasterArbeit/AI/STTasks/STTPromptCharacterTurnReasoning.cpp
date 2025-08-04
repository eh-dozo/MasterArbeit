#include "STTPromptCharacterTurnReasoning.h"

#include "LlamaCppSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "StateTree"

//TODO FOR SOME REASON IT'S CALLED EVERY TICK ? -> FIX
EStateTreeRunStatus FStateTreePromptCharacterTurnReasoningTask::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.ChatHistoryDataTable.IsValid()
		|| InstanceData.ChatHistoryDataTable->GetRowNames().Num() <= 0)
	{
		return EStateTreeRunStatus::Failed;
	}

	TArray<FChatMessage> ChatHistory;

	TArray<FChatMessage*> ChatHistoryDataTableRows;
	InstanceData.ChatHistoryDataTable->GetAllRows<FChatMessage>("", ChatHistoryDataTableRows);
	for (FChatMessage* Row : ChatHistoryDataTableRows)
	{
		if (Row)
		{
			ChatHistory.Add(*Row);
		}
	}

	FChatMessage LastUserPrompt = ChatHistory.Pop();
	if (LastUserPrompt.ChatRole != User)
	{
		return EStateTreeRunStatus::Failed;
	}

	bool bFoundAndExecuted = false;

	// can't use GameInstance->GetSubsystem<...> because it returns the abstract parent class and not the bp subclass
	UGameplayStatics::GetGameInstance(Context.GetOwner())->ForEachSubsystem<UGameInstanceSubsystem>(
		[&ChatHistory, &InstanceData, &LastUserPrompt, &bFoundAndExecuted](UGameInstanceSubsystem* Subsystem)
		{
			if (ULlamaCppSubsystem* LlamaRunnerSubsystem = Cast<ULlamaCppSubsystem>(Subsystem))
			{
				LlamaRunnerSubsystem->AddChatHistory(
					ChatHistory,
					InstanceData.SystemPromptDataAsset->SystemPrompt.ToString(),
					true);

				LlamaRunnerSubsystem->AsyncProcessUserPrompt(
					LastUserPrompt.Content,
					"");

				bFoundAndExecuted = true;

				// stop iterating once found
				return false;
			}

			// continue iterating
			return true;
		});

	if (!bFoundAndExecuted)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find LlamaCppSubsystem"));
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FStateTreePromptCharacterTurnReasoningTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
                                                                 const IStateTreeBindingLookup& BindingLookup,
                                                                 EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	check(InstanceData);

	const FText Format = (Formatting == EStateTreeNodeFormatting::RichText)
		                     ? LOCTEXT("PromptCharacterTurnReasoningRich", "<b>Prompt Model</> \"{Text}\"")
		                     : LOCTEXT("PromptCharacterTurnReasoning", "Prompt Model \"{Text}\"");

	return FText::FormatNamed(Format,
	                          TEXT("Text"), LOCTEXT("with Character narrative", "with Character narrative"));
}
#endif

#undef LOCTEXT_NAMESPACE
