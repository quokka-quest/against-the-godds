// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayEffectTurnStart.h"

UGameplayEffectTurnStart::UGameplayEffectTurnStart()
{
	// Make stacking global per target
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
}

