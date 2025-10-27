// Fill out your copyright notice in the Description page of Project Settings.


#include "ExecutionCalculationNewEffect.h"

#include "AbilitySystemComponent.h"

void UExecutionCalculationNewEffect::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	UAbilitySystemComponent* AbilitySystemComp = ExecutionParams.GetSourceAbilitySystemComponent();

	if (!AbilitySystemComp)
	{
		return;
	}

	
	
	// LOOP THROUGH THE EFFECT AND MAKE A NEW SPEC FOR EACH AND THEN APPLY IT TO THE PARENT
	for (TSubclassOf<UGameplayEffect> EffectClass : EffectsToApply)
	{
		if (EffectClass)
		{
			const FGameplayEffectContextHandle EffectContext = AbilitySystemComp->MakeEffectContext();
			const FGameplayEffectSpecHandle NewSpecHandle = AbilitySystemComp->MakeOutgoingSpec(EffectClass, 0.0f, EffectContext);
			if (NewSpecHandle.IsValid())
			{
				int CurrentIndex = 0;
				for (const FGameplayTag& Tag : EffectTags)
				{
					// Set the magnitude for each tag from the array
					NewSpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag, EffectMagnitudes[CurrentIndex]);
					
					// Increment the index but clamp it to the size of the array
					if (!CurrentIndex == EffectMagnitudes.Num() - 1)
					{
						CurrentIndex++;
					}
					
				}
				AbilitySystemComp->ApplyGameplayEffectSpecToSelf(*NewSpecHandle.Data.Get());
			}
		}
	}
}
