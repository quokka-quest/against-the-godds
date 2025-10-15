// Copright Notice: Publisher- James Bourne, Year of Publication- 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EventQueueSystem.h"

#include "EventQueueBlueprintHandler.generated.h"

/**
 * 
 */
UCLASS()
class EVENTQUEUESUBSYSTEM_API UEventQueueBlueprintHandler : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	static void EnqueueEvent(FQueuedEvent EventToCall, FInstancedStruct Payload, FName queueTag, FName eventTag, int32 priority, int32 quantity);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	static void ToggleQueuePauseState(FName queueTag, bool pause);

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem")
	static void DequeueEvent(FName queueTag, FName eventTag, bool removeAllEventsWithMatchingTag);
};
