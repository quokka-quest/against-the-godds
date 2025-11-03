// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnBasedAbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"

FActiveGameplayEffectHandle UTurnBasedAbilitySystemComponent::ApplyEffectWithInstantOnStackLoss(
    const FGameplayEffectSpecHandle& EffectSpecHandle, const FGameplayEffectSpecHandle& InstantEffectSpecHandle)
{
    // Validate the outgoing spec handle before applying
    if (!EffectSpecHandle.IsValid())
    {
        return FActiveGameplayEffectHandle();
    }

    // Apply the stacking effect using the spec from the handle
    FActiveGameplayEffectHandle Handle = ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

    if (Handle.IsValid())
    {
        // Store the instant effect spec handle to apply on stack loss (only if it's valid)
        if (InstantEffectSpecHandle.IsValid())
        {
            StackLossEffectMap.Add(Handle, InstantEffectSpecHandle);
        }

        // Bind to stack change event
        FOnActiveGameplayEffectStackChange* StackChangeDelegate = OnGameplayEffectStackChangeDelegate(Handle);
        if (StackChangeDelegate)
        {
            StackChangeDelegate->AddLambda([this](FActiveGameplayEffectHandle EffectHandle, int32 NewStackCount, int32 PreviousStackCount)
            {
                OnStackCountChanged(EffectHandle, NewStackCount, PreviousStackCount);
            });
        }

        // Bind to removal event to handle last stack removal
        FOnActiveGameplayEffectRemoved_Info* RemovalDelegate = OnGameplayEffectRemoved_InfoDelegate(Handle);
        
        if (RemovalDelegate)
        {
            RemovalDelegate->AddLambda([this](const FGameplayEffectRemovalInfo& RemovalInfo)
            {
                OnEffectRemoved(RemovalInfo.ActiveEffect->Handle);
            });
        }
    }

    return Handle;
}


void UTurnBasedAbilitySystemComponent::OnStackCountChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount)
{
    // Check if stack decreased
    if (NewStackCount < PreviousStackCount)
    {
        if (FGameplayEffectSpecHandle* SpecHandlePtr = StackLossEffectMap.Find(Handle))
        {
            const FGameplayEffectSpecHandle& InstantSpecHandle = *SpecHandlePtr;
            if (InstantSpecHandle.IsValid() && InstantSpecHandle.Data.IsValid())
            {
                // Apply instant effect for each stack lost
                int32 StacksLost = PreviousStackCount - NewStackCount;
                for (int32 i = 0; i < StacksLost; ++i)
                {
                    ApplyGameplayEffectSpecToSelf(*InstantSpecHandle.Data.Get());
                }
            }
        }
    }

    // Clean up if effect is fully removed
    if (NewStackCount == 0)
    {
        StackLossEffectMap.Remove(Handle);
    }
}

void UTurnBasedAbilitySystemComponent::OnEffectRemoved(FActiveGameplayEffectHandle Handle)
{
    if (FGameplayEffectSpecHandle* SpecHandlePtr = StackLossEffectMap.Find(Handle))
    {
        const FGameplayEffectSpecHandle& InstantSpecHandle = *SpecHandlePtr;
        if (InstantSpecHandle.IsValid() && InstantSpecHandle.Data.IsValid())
        {
            // Apply instant effect one final time for the last stack
            ApplyGameplayEffectSpecToSelf(*InstantSpecHandle.Data.Get());
        }
    }
    
    // Clean up the mapping
    StackLossEffectMap.Remove(Handle);
}

