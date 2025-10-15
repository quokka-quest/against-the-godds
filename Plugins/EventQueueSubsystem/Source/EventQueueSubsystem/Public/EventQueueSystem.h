// Copright Notice: Publisher- James Bourne, Year of Publication- 2025

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Queue.h"
#include "StructUtils/InstancedStruct.h"
#include "TimerManager.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "EventQueueSystem.generated.h"

// This delegate is for holding the event information for passing into the EnqueueEvent function (means only events that have the 'FInstancedStruct' parameter can be passed in
DECLARE_DYNAMIC_DELEGATE_OneParam(FQueuedEvent, FInstancedStruct, Payload);

// These delegates are for binding events to in blueprints.
// a multicast delegate can have multiple functions bound to it
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventQueued, FName, QueueTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueFinished, FName, QueueTag);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEventCompleted, FGuid);

// The struct for holding event information while it is in the queue.
// Priority and sequence number are for sorting the events
// payload is the data to be passed into the event as a parameter
USTRUCT()
struct FQueuedEventData
{
	GENERATED_BODY()

	FInstancedStruct payload;
	FQueuedEvent Event;
	int32 priority;
	int64 sequenceNumber;
	FName eventTag;
	FGuid uniqueID;
};

// This struct is for holding the queues and their information
USTRUCT()
struct FNamedQueue
{
	GENERATED_BODY()
	
	TArray<FQueuedEventData> EventQueue;
	bool processingEvent = false;
	bool isPaused = false;
	FName tag;
	int32 maxLength;
	FGuid currentEventID;
	float delayBetweenEvents;
};

USTRUCT(Blueprintable, Category = "EventQueueSystem")
struct FQueueState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EventQueueSystem")
	int32 length = 0;
	UPROPERTY(BlueprintReadOnly, Category = "EventQueueSystem")
	int32 maxLength = 0;
	UPROPERTY(BlueprintReadOnly, Category = "EventQueueSystem")
	bool isPaused = false;
	UPROPERTY(BlueprintReadOnly, Category = "EventQueueSystem")
	bool isProcessing = false;
};

/**
 * 
 */
UCLASS()
class EVENTQUEUESUBSYSTEM_API UEventQueueSystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void EnqueueEvent(FQueuedEvent EventToCall, FInstancedStruct Payload, FName queueTag, FName eventTag, int32 priority, int32 quantity);
	void EnqueueWaitEvent(FName queueTag, FQueuedEventData eventData, int32 quantity);
	void PauseQueue(FName queueTag);
	void ResumeQueue(FName queueTag);
	
	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem", meta=(WorldContext = "WorldContextObject"))
	void MarkEventAsComplete(FName queueTag, UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	void CreateQueue(FName queueTag, bool startPaused);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	void EmptyQueue(FName queueTag);

	void DequeueEvent(FName queueTag, FName eventTag, bool removeAllEventsWithMatchingTag);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	void SetQueueMaxLength(FName queueTag, int32 maxLength, bool truncateQueue);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	void ToggleQueueSystemPauseState(bool Paused);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	void FlushAllQueues();

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	void SetQueueProcessDelay(FName queueTag, float delayBetweenEvents);

	UFUNCTION(BlueprintPure, Category = "EventQueueSystem")
	FQueueState GetQueueState(FName queueTag);

	UFUNCTION(BlueprintPure, Category = "EventQueueSystem")
	TArray<FName> GetAllQueueTags();

	UFUNCTION(BlueprintPure, Category = "EventQueueSystem")
	bool GetGlobalPauseState();

	UPROPERTY(BlueprintAssignable, Category = "EventQueueSystem")
	FOnQueueFinished OnQueueFinished;

	UPROPERTY(BlueprintAssignable, Category = "EventQueueSystem")
	FOnEventQueued OnEventQueued;

	// not blueprint callable since this is for the 'enqueue and wait node' class to use
	FOnEventCompleted OnEventCompleted;
	int64 queueSequenceNumber = 0;

private:
	void ProcessQueue(FName queueTag);
	void ResetQueueSystemState();
	void EndPIEFunction(bool bSimulate);
	void WorldCleanupFunction(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	
	TMap<FName, FNamedQueue> EventQueueMap;
	bool globalPaused = false;
	FTimerHandle TimerHandle;
};
