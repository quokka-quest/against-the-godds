// Copright Notice: Publisher- James Bourne, Year of Publication- 2025

#pragma once

#include "EventQueueSystem.h"
#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "InstancedStruct.h"
#include "Subsystems/EngineSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "EnqueueAndWaitNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEventComplete);

UCLASS()
class EVENTQUEUESUBSYSTEM_API UEnqueueEventAndWaitNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "EventQueueSystem")
	FOnEventComplete OnComplete;

	UFUNCTION(BlueprintCallable, Category = "EventQueueSystem", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UEnqueueEventAndWaitNode* EnqueueEventAndWait(UObject* WorldContextObject, FQueuedEvent EventToCall, FInstancedStruct Payload, FName queueTag, FName eventTag, int32 priority, int32 quantity);

	virtual void Activate() override;

private:
	FGuid eventID;
	int32 quantityToWaitOn;

	void EventCompleteCallback(FGuid completedEventID);
};
