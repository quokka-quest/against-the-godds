// Copright Notice: Publisher- James Bourne, Year of Publication- 2025


#include "EventQueueBlueprintHandler.h"
#include "Engine/Engine.h"

void UEventQueueBlueprintHandler::EnqueueEvent(FQueuedEvent EventToCall, FInstancedStruct Payload, FName queueTag, FName eventTag, int32 priority, int32 quantity)
{
	UEventQueueSystem* queueSystem = GEngine->GetEngineSubsystem<UEventQueueSystem>();
	queueSystem->EnqueueEvent(EventToCall, Payload, queueTag, eventTag, priority, quantity);
}

void UEventQueueBlueprintHandler::ToggleQueuePauseState(FName queueTag, bool pause)
{
	UEventQueueSystem* queueSystem = GEngine->GetEngineSubsystem<UEventQueueSystem>();
	if (pause)
	{
		queueSystem->PauseQueue(queueTag);
	}
	else
	{
		queueSystem->ResumeQueue(queueTag);
	}
}

void UEventQueueBlueprintHandler::DequeueEvent(FName queueTag, FName eventTag, bool removeAllEventsWithMatchingTag)
{
	UEventQueueSystem* queueSystem = GEngine->GetEngineSubsystem<UEventQueueSystem>();
	queueSystem->DequeueEvent(queueTag, eventTag, removeAllEventsWithMatchingTag);
}
