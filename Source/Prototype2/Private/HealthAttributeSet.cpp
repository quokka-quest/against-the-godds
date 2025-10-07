// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetMaxHealth(FMath::Max(GetMaxHealth(), 1.0f));
	}
}
