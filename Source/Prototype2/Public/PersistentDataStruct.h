// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityBase.h"

struct FPersistentPlayerInfo
{
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
	TMap<TSubclassOf<UGameplayAbilityBase>, FDiceFaceLevels> AbilityDiceMap;
	TArray<TSubclassOf<UGameplayEffect>> ActiveEffects;

	// stats
	int MaxMovement;
	int MaxAttacks;
	
	float MaxHealth;
	float CurrentHealth;
};
