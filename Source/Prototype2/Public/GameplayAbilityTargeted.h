// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityBase.h"
#include "GameplayAbilityTargeted.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UGameplayAbilityTargeted : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<AActor*> GetTargets() const;
	
};
