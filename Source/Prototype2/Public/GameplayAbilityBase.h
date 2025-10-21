// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilityBase.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual float CalculateDamageWithMods(float BaseDamage);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void RollDice();
};
