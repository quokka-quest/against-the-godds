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
	AEnemyEntity();

	UPROPERTY(BlueprintReadWrite, Category = "EnemyLogic")
	APlayerEntity* PlayerTarget;

protected:
	
	virtual UStaticMesh* GetCharMesh() const { return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Static_Meshes/Enemies/SM_Wolf.SM_Wolf")); }

	UFUNCTION(BlueprintCallable)
	void DeterminePlayerTarget();

	UFUNCTION(BlueprintCallable)
	void DetermineMovement();

	UFUNCTION(BlueprintCallable)
	void DetermineAttack();
};
