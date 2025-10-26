// Fill out your copyright notice in the Description page of Project Settings.


#include "GridCell.h"

// Sets default values
AGridCell::AGridCell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsOccupied = false;
	OccupyingEntity = nullptr;
	isWalkable = false;

}

void AGridCell::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FVector pos = FVector(
		GridCellCoord.X * cellSize,
		GridCellCoord.Y * cellSize,
		GridCellCoord.Z * cellHeight
		);
	SetActorLocation(pos);
}


void AGridCell::QueryIfTileIsWalkable(AGridCell* FromCell, int CostToReach)
{
	if (!IsOccupied)
	{
		int HeightDiff = abs(FromCell->GridCellCoord.Z - GridCellCoord.Z);
		if (HeightDiff == 1 || HeightDiff == 0)
		{
			isWalkable = true;
			MovementCost = CostToReach;
		}
	}
}


