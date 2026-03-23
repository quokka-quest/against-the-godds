// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnBasedAbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"

FActiveGameplayEffectHandle UTurnBasedAbilitySystemComponent::BindGameplayEffectToOnStackLoss(
    const FGameplayEffectSpecHandle& EffectSpecHandle, const FGameplayEffectSpecHandle& InstantEffectSpecHandle, bool bApplyPerStack)
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
        // Extract effect classes from the spec handles
        TSubclassOf<UGameplayEffect> EffectClass = nullptr;
        TSubclassOf<UGameplayEffect> InstantEffectClass = nullptr;

        if (EffectSpecHandle.Data.IsValid() && EffectSpecHandle.Data->Def)
        {
            EffectClass = EffectSpecHandle.Data->Def->GetClass();
        }

        if (InstantEffectSpecHandle.IsValid() && InstantEffectSpecHandle.Data.IsValid() && InstantEffectSpecHandle.Data->Def)
        {
            InstantEffectClass = InstantEffectSpecHandle.Data->Def->GetClass();
        }

        int32 InitialStackCount = 1;
        if (const FActiveGameplayEffect* ActiveEffect = GetActiveGameplayEffect(Handle))
        {
            InitialStackCount = FMath::Max(ActiveEffect->Spec.GetStackCount(), 1);
        }

        // If we already have this handle tracked, just update its data and avoid rebinding delegates
        if (StackLossEffectMap.Contains(Handle))
        {
            FStackLossEffectData* ExistingData = StackLossEffectMap.Find(Handle);
            if (ExistingData)
            {
                // Update the instant spec and per-stack flag (if a valid instant spec is provided)
                if (InstantEffectSpecHandle.IsValid())
                {
                    ExistingData->InstantEffectSpecHandle = InstantEffectSpecHandle;
                    ExistingData->InstantEffectClass = InstantEffectClass;
                    ExistingData->bFinalStackLossHandled = false;
                }
                ExistingData->bApplyPerStack = bApplyPerStack;
                ExistingData->LastKnownStackCount = InitialStackCount;
            }
        }
        else
        {
            // Store the instant effect spec handle to apply on stack loss (only if it's valid)
            if (InstantEffectSpecHandle.IsValid())
            {
                StackLossEffectMap.Add(Handle, {InstantEffectSpecHandle, EffectClass, InstantEffectClass, bApplyPerStack, false, InitialStackCount});
            }
            else
            {
                // Add with an invalid InstantEffectSpecHandle; allows future updates without rebinding
                StackLossEffectMap.Add(Handle, {FGameplayEffectSpecHandle(), EffectClass, InstantEffectClass, bApplyPerStack, false, InitialStackCount});
            }

            // Bind to stack change event ONCE per active effect handle
            FOnActiveGameplayEffectStackChange* StackChangeDelegate = OnGameplayEffectStackChangeDelegate(Handle);
            if (StackChangeDelegate)
            {
                StackChangeDelegate->AddLambda([this](FActiveGameplayEffectHandle EffectHandle, int32 NewStackCount, int32 PreviousStackCount)
                {
                    OnStackCountChanged(EffectHandle, NewStackCount, PreviousStackCount);
                });
            }

            // Bind to removal event ONCE to handle last stack removal
            FOnActiveGameplayEffectRemoved_Info* RemovalDelegate = OnGameplayEffectRemoved_InfoDelegate(Handle);
            
            if (RemovalDelegate)
            {
                RemovalDelegate->AddLambda([this](const FGameplayEffectRemovalInfo& RemovalInfo)
                {
                      if (RemovalInfo.ActiveEffect)
                      { 
                          OnEffectRemoved(RemovalInfo.ActiveEffect->Handle);
                      }
                });
            }
        }
    }

    return Handle;
}


void UTurnBasedAbilitySystemComponent::OnStackCountChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount)
{
    UE_LOG(LogTemp, Log, TEXT("OnStackCountChanged called: NewStackCount=%d, PreviousStackCount=%d"), NewStackCount, PreviousStackCount);

    if (FStackLossEffectData* EffectData = StackLossEffectMap.Find(Handle))
    {
        EffectData->LastKnownStackCount = NewStackCount;
    }

  const int32 RemovedStackCount = FMath::Max(PreviousStackCount - NewStackCount, 0);
  if (RemovedStackCount > 0)
    {
        if (FStackLossEffectData* EffectData = StackLossEffectMap.Find(Handle))
        {
            const FGameplayEffectSpecHandle& InstantSpecHandle = EffectData->InstantEffectSpecHandle;
            if (InstantSpecHandle.IsValid() && InstantSpecHandle.Data.IsValid())
            {
                if (EffectData->bApplyPerStack)
                {
          // Apply once for each stack that was actually removed.
          for (int32 i = 0; i < RemovedStackCount; ++i)
                    {
                        ApplyGameplayEffectSpecToSelf(*InstantSpecHandle.Data.Get());
                    }
                }
                else
                {
                    // Apply once for any decrease
                    ApplyGameplayEffectSpecToSelf(*InstantSpecHandle.Data.Get());
                }
            }

                  if (NewStackCount == 0)
                  {
                    EffectData->bFinalStackLossHandled = true;
                  }
        }
    }
}

void UTurnBasedAbilitySystemComponent::OnEffectRemoved(FActiveGameplayEffectHandle Handle)
{
    if (FStackLossEffectData* EffectData = StackLossEffectMap.Find(Handle))
    {
        const FGameplayEffectSpecHandle& InstantSpecHandle = EffectData->InstantEffectSpecHandle;
        TSubclassOf<UGameplayEffect> EffectClass = EffectData->EffectClass;
        TSubclassOf<UGameplayEffect> InstantEffectClass = EffectData->InstantEffectClass;

        // If the stack-change callback did not report the final removal, apply using last known stack count.
        if (!EffectData->bFinalStackLossHandled && InstantSpecHandle.IsValid() && InstantSpecHandle.Data.IsValid())
        {
            if (EffectData->bApplyPerStack)
            {
                const int32 MissedRemovedStackCount = FMath::Max(EffectData->LastKnownStackCount, 1);
                for (int32 i = 0; i < MissedRemovedStackCount; ++i)
                {
                    ApplyGameplayEffectSpecToSelf(*InstantSpecHandle.Data.Get());
                }
            }
            else
            {
                ApplyGameplayEffectSpecToSelf(*InstantSpecHandle.Data.Get());
            }
        }

        // Broadcast the event when the final stack is removed with the cached effect classes
        OnFinalStackRemoved.Broadcast(EffectClass, InstantEffectClass);
    }
    
    // Clean up the mapping
    StackLossEffectMap.Remove(Handle);
}
