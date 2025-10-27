// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "TurnBasedAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UTurnBasedAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()


public:
	// Apply a stacking gameplay effect and register to apply `InstantEffectToApply` whenever a stack is removed.
	UFUNCTION(BlueprintCallable, Category = "TurnBasedAbilitySystemComponent")
	FActiveGameplayEffectHandle ApplyEffectWithInstantOnStackLoss(const FGameplayEffectSpecHandle& EffectSpecHandle, const FGameplayEffectSpecHandle& InstantEffectSpecHandle);

protected:
	// Callback for when a stack count changes
	void OnStackCountChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount);
	void OnEffectRemoved(FActiveGameplayEffectHandle Handle);

private:
	// Maps active effect handles to instant effects that should be applied on stack loss
	TMap<FActiveGameplayEffectHandle, FGameplayEffectSpecHandle> StackLossEffectMap;
};
