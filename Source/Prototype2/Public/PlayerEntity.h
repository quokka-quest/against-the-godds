// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntityBase.h"
#include "PlayerEntity.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API APlayerEntity : public AEntityBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="PlayerInfo")
	void SendStatsToGameInstance(TSubclassOf<APlayerEntity> CharClass);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="PlayerInfo")
	void UpdateGlobalStatData();
};
