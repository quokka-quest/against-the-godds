// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityBase.h"
#include "PersistentDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FPersistentPlayerInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "PlayerSave")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
	UPROPERTY(BlueprintReadWrite, Category = "PlayerSave")
	TMap<TSubclassOf<UGameplayAbilityBase>, FDiceFaceLevels> AbilityDiceMap;
	UPROPERTY(BlueprintReadWrite, Category = "PlayerSave")
	TArray<TSubclassOf<UGameplayEffect>> ActiveEffects;

	// stats
	UPROPERTY(BlueprintReadWrite, Category = "PlayerSave")
	int MaxMovement;
	UPROPERTY(BlueprintReadWrite, Category="PlayerSave")
	int MaxAttacks;
	
	UPROPERTY(BlueprintReadWrite, Category = "PlayerSave")
	float MaxHealth;
	UPROPERTY(BlueprintReadWrite, Category="PlayerSave")
	float CurrentHealth;
};
