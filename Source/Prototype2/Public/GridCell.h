// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyEntity.h"
#include "GridCell.generated.h"

UCLASS()
class PROTOTYPE2_API AGridCell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridCell();

	UFUNCTION(BlueprintCallable)
	void QueryIfTileIsWalkable(AGridCell* FromCell, int CostToReach);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FIntVector GridCellCoord;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool IsPlayerSpawnTile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool IsEnviroHazardTile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool IsEnemySpawnTile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<AEnemyEntity> EnemyToSpawn;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int MovementCost;

	UPROPERTY(BlueprintReadWrite, Category = "Grid")
	bool IsOccupied;
	UPROPERTY(BlueprintReadWrite, Category = "Grid")
	AEntityBase* OccupyingEntity;
	UPROPERTY(BlueprintReadWrite, Category = "Grid")
	bool isWalkable;

private:
	float cellSize = 200.0f;
	float cellHeight = 50.0f;

protected:

	virtual void OnConstruction(const FTransform& Transform) override;
};
