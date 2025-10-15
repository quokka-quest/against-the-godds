// Copright Notice: Publisher- James Bourne, Year of Publication- 2025

#include "EnqueueAndWaitNode.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UEnqueueEventAndWaitNode* UEnqueueEventAndWaitNode::EnqueueEventAndWait(UObject* WorldContextObject, FQueuedEvent EventToCall, FInstancedStruct Payload, FName queueTag, FName eventTag, int32 priority, int32 quantity)
{
	UEnqueueEventAndWaitNode* node = NewObject<UEnqueueEventAndWaitNode>();
	quantity = (quantity > 0) ? quantity : 1;
	node->eventID = FGuid::NewGuid();
	node->quantityToWaitOn = quantity;

	if (UEventQueueSystem* queue = GEngine->GetEngineSubsystem<UEventQueueSystem>())
	{
		queue->OnEventCompleted.AddUObject(node, &UEnqueueEventAndWaitNode::EventCompleteCallback);

		FQueuedEventData newEvent;
		newEvent.Event = EventToCall;
		newEvent.payload = Payload;
		newEvent.priority = priority;
		newEvent.eventTag = eventTag;
		newEvent.uniqueID = node->eventID;
		newEvent.sequenceNumber = queue->queueSequenceNumber;
		queue->queueSequenceNumber++;

		queue->EnqueueWaitEvent(queueTag, newEvent, quantity);
	}

	return node;
}

void UEnqueueEventAndWaitNode::EventCompleteCallback(FGuid completedEventID)
{
	if (completedEventID != eventID) return;
	quantityToWaitOn--;
	if (quantityToWaitOn != 0) return;

	
	if (UEventQueueSystem* queue = GEngine->GetEngineSubsystem<UEventQueueSystem>())
	{
		queue->OnEventCompleted.RemoveAll(this);
	}
	OnComplete.Broadcast();
}

void UEnqueueEventAndWaitNode::Activate() { }






