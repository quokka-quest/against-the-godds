// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "TurnBasedAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFinalStackRemovedDelegate, TSubclassOf<UGameplayEffect>, EffectClass, TSubclassOf<UGameplayEffect>, InstantEffectClass);

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
	FActiveGameplayEffectHandle BindGameplayEffectToOnStackLoss(const FGameplayEffectSpecHandle& EffectSpecHandle, const FGameplayEffectSpecHandle& InstantEffectSpecHandle, bool bApplyPerStack = true);

protected:
	// Callback for when a stack count changes
	void OnStackCountChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount);
	void OnEffectRemoved(FActiveGameplayEffectHandle Handle);
	

private:
	struct FStackLossEffectData
	{
		FGameplayEffectSpecHandle InstantEffectSpecHandle;
		TSubclassOf<UGameplayEffect> EffectClass;
		TSubclassOf<UGameplayEffect> InstantEffectClass;
		bool bApplyPerStack = true;
		bool bFinalStackLossHandled = false;
		int32 LastKnownStackCount = 1;
	};

	// Maps active effect handles to instant effects that should be applied on stack loss
	TMap<FActiveGameplayEffectHandle, FStackLossEffectData> StackLossEffectMap;
};
