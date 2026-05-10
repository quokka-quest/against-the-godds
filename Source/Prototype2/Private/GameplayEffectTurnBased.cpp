// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffectTurnBased.h"

UGameplayEffectTurnBased::UGameplayEffectTurnBased()
{
	// Make stacking global per target
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
}

