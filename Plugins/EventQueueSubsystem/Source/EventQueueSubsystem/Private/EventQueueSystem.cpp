// Copright Notice: Publisher- James Bourne, Year of Publication- 2025


#include "EventQueueSystem.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

// performs cleanup on initialisation
void UEventQueueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetQueueSystemState();

	FWorldDelegates::OnWorldCleanup.AddUObject(this,&UEventQueueSystem::WorldCleanupFunction);

#if WITH_EDITOR
	FEditorDelegates::EndPIE.AddUObject(this, &UEventQueueSystem::EndPIEFunction);
#endif
	
	UE_LOG(LogTemp, Display, TEXT("EventQueueSystem Initialized"));
}

// performs cleanup on de-initialisation
void UEventQueueSystem::Deinitialize()
{
	Super::Deinitialize();

	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

#if WITH_EDITOR
	FEditorDelegates::EndPIE.RemoveAll(this);
#endif
	
	UE_LOG(LogTemp, Display, TEXT("EventQueueSystem Deinitialized"));
}

// Called by the user in blueprints. This adds an event to the queue
void UEventQueueSystem::EnqueueEvent(FQueuedEvent EventToCall, FInstancedStruct Payload, FName queueTag, FName eventTag, int32 priority, int32 quantity)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Tried to ENQUEUE an event to a queue with the tag '%s' which does not exist"), *queueTag.ToString());
		return;
	}

	quantity = (quantity > 0) ? quantity : 1;
	if (EventQueueMap[queueTag].EventQueue.Num() + quantity > EventQueueMap[queueTag].maxLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Tried to ENQUEUE an event to a queue with the tag '%s' which will exceed the set queue capacity"), *queueTag.ToString());
		return;
	}
	
	FQueuedEventData NewEvent;
	NewEvent.payload = Payload;
	NewEvent.Event = EventToCall;
	NewEvent.priority = priority;
	NewEvent.eventTag = eventTag;
	NewEvent.uniqueID = FGuid::NewGuid();
	NewEvent.sequenceNumber = queueSequenceNumber;
	queueSequenceNumber++;

	for (int i = 0; i < quantity; i++)
	{
		EventQueueMap[queueTag].EventQueue.Add(NewEvent);
		OnEventQueued.Broadcast(queueTag);
	}

	// This sort means events are organised by priority number (lower values first)
	// If events have the same priority then they are sorted based on when they were added to the queue
	EventQueueMap[queueTag].EventQueue.Sort([](const FQueuedEventData& A, const FQueuedEventData& B)
	{
		if (A.priority == B.priority)
		{
			return A.sequenceNumber < B.sequenceNumber;
		}
		return A.priority < B.priority;
	});

	ProcessQueue(queueTag);
}

void UEventQueueSystem::EnqueueWaitEvent(FName queueTag, FQueuedEventData eventData, int32 quantity)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Tried to ENQUEUE AND WAIT an event to a queue with the tag '%s' which does not exist"), *queueTag.ToString());
		return;
	}

	if (EventQueueMap[queueTag].EventQueue.Num() >= EventQueueMap[queueTag].maxLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Tried to ENQUEUE AND WAIT an event to a queue with the tag '%s' which has reached the set queue capacity"), *queueTag.ToString());
		return;
	}

	for (int i = 0; i < quantity; i++)
	{
		EventQueueMap[queueTag].EventQueue.Add(eventData);
		OnEventQueued.Broadcast(queueTag);
	}

	EventQueueMap[queueTag].EventQueue.Sort([](const FQueuedEventData& A, const FQueuedEventData& B)
	{
		if (A.priority == B.priority)
		{
			return A.sequenceNumber < B.sequenceNumber;
		}
		return A.priority < B.priority;
	});
	
	ProcessQueue(queueTag);
}

// automatically called when the queue gains an entry of when the current event has finished
// executes the next event in the queue
void UEventQueueSystem::ProcessQueue(FName queueTag)
{
	FNamedQueue temp = EventQueueMap[queueTag];
	if (temp.EventQueue.IsEmpty()) OnQueueFinished.Broadcast(queueTag);
	if (temp.processingEvent|| temp.EventQueue.IsEmpty() || temp.isPaused || globalPaused) return;
	
	EventQueueMap[queueTag].processingEvent = true;
	FQueuedEventData Next = EventQueueMap[queueTag].EventQueue[0];
	EventQueueMap[queueTag].currentEventID = Next.uniqueID;
	EventQueueMap[queueTag].EventQueue.RemoveAt(0);
	
	if (Next.Event.IsBound())
	{
		Next.Event.Execute(Next.payload);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Queue with the tag '%s' contained an item but the item's event was invalid"), *queueTag.ToString());
		EventQueueMap[queueTag].processingEvent = false;
	}
}

// called manually in blueprints by the user on the finish of an event
// This is needed so that asynchronous events can be used in the queue system
void UEventQueueSystem::MarkEventAsComplete(FName queueTag, UObject* WorldContextObject)
{
	if (!EventQueueMap.Contains(queueTag)) return;
	
	OnEventCompleted.Broadcast(EventQueueMap[queueTag].currentEventID);

	if (EventQueueMap[queueTag].delayBetweenEvents == 0.0f)
	{
		EventQueueMap[queueTag].processingEvent = false;
		ProcessQueue(queueTag);
		return;
	}

	// delay
	WorldContextObject->GetWorld()->GetTimerManager().SetTimerForNextTick([this, queueTag, WorldContextObject]()
	{
		WorldContextObject->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, queueTag]()
		{
			EventQueueMap[queueTag].processingEvent = false;
			ProcessQueue(queueTag);
		}, EventQueueMap[queueTag].delayBetweenEvents,false);
	});
}

// This function is for cleaning up the subsystem so that it works
// Resets the variables related to the queue state
void UEventQueueSystem::ResetQueueSystemState()
{
	// reset variables
	EventQueueMap.Empty();
	queueSequenceNumber = 0;
	globalPaused = false;

	// create default queue
	FNamedQueue temp;
	temp.tag = "None";
	temp.maxLength = MAX_int32;
	temp.delayBetweenEvents = 0.0f;
	EventQueueMap.Add("None",temp);
}

// cleans up the subsystem after a run of the program (editor only)
void UEventQueueSystem::EndPIEFunction(bool bSimulate)
{
	ResetQueueSystemState();
}

// cleans up the subsystem after a run of the game (in a game build)
void UEventQueueSystem::WorldCleanupFunction(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	ResetQueueSystemState();
}

// pause a specific queue
void UEventQueueSystem::PauseQueue(FName queueTag)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to PAUSE a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
		return;
	}

	EventQueueMap[queueTag].isPaused = true;
}

// unpause a specific queue
void UEventQueueSystem::ResumeQueue(FName queueTag)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to UNPAUSE a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
		return;
	}

	EventQueueMap[queueTag].isPaused = false;
	if (EventQueueMap[queueTag].EventQueue.IsEmpty()) return;
	
	ProcessQueue(queueTag);
}

// create a queue with the given tag and pause state
void UEventQueueSystem::CreateQueue(FName queueTag, bool startPaused)
{
	if (EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to CREATE a queue with the tag '%s' which already exists"), *queueTag.ToString());
		return;
	}
	
	FNamedQueue temp;
	temp.tag = queueTag;
	temp.isPaused = startPaused;
	temp.maxLength = MAX_int32;
	temp.delayBetweenEvents = 0.0f;
	EventQueueMap.Add(queueTag,temp);
}

void UEventQueueSystem::EmptyQueue(FName queueTag)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to EMPTY a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
		return;
	}

	EventQueueMap[queueTag].EventQueue.Empty();
	EventQueueMap[queueTag].processingEvent = false;
}

// Dequeues events with the given eventTag from the queue with the given queueTag
// has a boolean to specify if all events with matching tags should be dequeued. If its false then only the first one is removed
void UEventQueueSystem::DequeueEvent(FName queueTag, FName eventTag, bool removeAllEventsWithMatchingTag)
{
	if (eventTag == "None")
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Can not dequeue events with the tag 'None'"));
		return;
	}
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to DEQUEUE an event from a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
		return;
	}

	for (int i = 0; i < EventQueueMap[queueTag].EventQueue.Num(); i++)
	{
		if (EventQueueMap[queueTag].EventQueue[i].eventTag == eventTag)
		{
			EventQueueMap[queueTag].EventQueue.RemoveAt(i);
			if (!removeAllEventsWithMatchingTag) return;
		}
	}
}

// Sets the maximum number of items that can be in the queue with the given tag
// has a truncateQueue boolean to allow for removing all items currently from the queue that are past the new limit
void UEventQueueSystem::SetQueueMaxLength(FName queueTag, int32 maxLength, bool truncateQueue)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to SET QUEUE LENGTH of a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
		return;
	}

	EventQueueMap[queueTag].maxLength = maxLength;
	if (!truncateQueue || EventQueueMap[queueTag].EventQueue.Num() <= maxLength) return;

	int end = EventQueueMap[queueTag].EventQueue.Num();
	for (int i = maxLength; i < end; i++)
	{
		EventQueueMap[queueTag].EventQueue.Pop();
	}
}

void UEventQueueSystem::ToggleQueueSystemPauseState(bool Paused)
{
	globalPaused = Paused;

	if (Paused) return;

	for (TPair<FName, FNamedQueue> pair : EventQueueMap)
	{
		// this runs a queue if it contains an item (prevents the queue complete from firing when pausing/un-pausing)
		if (!pair.Value.EventQueue.IsEmpty()) ProcessQueue(pair.Key);
	}
}

FQueueState UEventQueueSystem::GetQueueState(FName queueTag)
{
	FQueueState temp;
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to GET QUEUE STATE of a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
		return temp;
	}

	temp.length = EventQueueMap[queueTag].EventQueue.Num();
	temp.maxLength = EventQueueMap[queueTag].maxLength;
	temp.isPaused = EventQueueMap[queueTag].isPaused;
	temp.isProcessing = EventQueueMap[queueTag].processingEvent;
	return temp;
}

void UEventQueueSystem::FlushAllQueues()
{
	for (TPair<FName, FNamedQueue> pair : EventQueueMap)
	{
		EventQueueMap[pair.Key].EventQueue.Empty();
	}
}

TArray<FName> UEventQueueSystem::GetAllQueueTags()
{
	TArray<FName> temp;
	EventQueueMap.GetKeys(temp);
	return temp;
}

bool UEventQueueSystem::GetGlobalPauseState()
{
	return globalPaused;
}

void UEventQueueSystem::SetQueueProcessDelay(FName queueTag, float delayBetweenEvents)
{
	if (!EventQueueMap.Contains(queueTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Attempted to SET QUEUE DELAY of a queue with the tag '%s' which doesn't exist"), *queueTag.ToString());
	}

	EventQueueMap[queueTag].delayBetweenEvents = delayBetweenEvents;
}



