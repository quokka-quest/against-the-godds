// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridCellBase.h"
#include "EnemyEntity.h"
#include "GridCellParent.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AGridCellParent : public AGridCellBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	bool IsPlayerSpawnCell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	bool IsEnemySpawnTile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	TSubclassOf<AEnemyEntity> EnemyToSpawn;
	
};
