// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "TurnBasedAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFinalStackRemovedDelegate, TSubclassOf<UGameplayEffect>, EffectClass, TSubclassOf<UGameplayEffect>, InstantEffectClass);

USTRUCT(BlueprintType)
struct PROTOTYPE2_API FStackLossEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayEffectSpecHandle InstantEffectSpecHandle;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> EffectClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> InstantEffectClass;
	
	UPROPERTY(EditAnywhere)
	bool bApplyPerStack = true;

	UPROPERTY(EditAnywhere)
	bool bLoopEffectForTotalStackCount = false;

	UPROPERTY(EditAnywhere)
	bool bFinalStackLossHandled = false;

	UPROPERTY(EditAnywhere)
	int32 LastKnownStackCount = 1;
};

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UTurnBasedAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()


public:
	// Blueprint event that fires when the final stack is removed
	UPROPERTY(BlueprintAssignable, Category = "TurnBasedAbilitySystemComponent")
	FOnFinalStackRemovedDelegate OnFinalStackRemoved;

	// Apply a stacking gameplay effect and register to apply `InstantEffectToApply` whenever a stack is removed.
	UFUNCTION(BlueprintCallable, Category = "TurnBasedAbilitySystemComponent")
	FActiveGameplayEffectHandle BindGameplayEffectToOnStackLoss(const FGameplayEffectSpecHandle& EffectSpecHandle, const FGameplayEffectSpecHandle& InstantEffectSpecHandle, bool bApplyPerStackLost = true, bool bLoopEffectForTotalStackCount = false);

	UFUNCTION(BlueprintCallable, Category = "TurnBasedAbilitySystemComponent")
	void SetUseOfStackLossEffect(bool EnableEffect);
private:
	
protected:
	// Callback for when a stack count changes
	void OnStackCountChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount);
	void OnEffectRemoved(FActiveGameplayEffectHandle Handle);

	// Maps active effect handles to instant effects that should be applied on stack loss
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TurnBasedAbilitySystemComponent")
	TMap<FActiveGameplayEffectHandle, FStackLossEffectData> StackLossEffectMap;

	UPROPERTY(BlueprintReadWrite, Category = "TurnBasedAbilitySystemComponent")
	bool bDontTriggerStackLossEffect;
	
	
	
};
