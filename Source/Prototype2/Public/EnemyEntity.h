// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntityBase.h"
#include "PlayerEntity.h"
#include "EnemyEntity.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AEnemyEntity : public AEntityBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "EnemyLogic")
	APlayerEntity* PlayerTarget;

protected:
	UFUNCTION(BlueprintCallable)
	void DeterminePlayerTarget();

	UFUNCTION(BlueprintCallable)
	void DetermineMovement();

	UFUNCTION(BlueprintCallable)
	bool DetermineAttack();

	bool IsTargetInAttackRange(int Range);

	void ChangeOccupancy(FIntVector2 Coord, bool SetAsOccupier);
};
