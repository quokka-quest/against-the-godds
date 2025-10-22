// Fill out your copyright notice in the Description page of Project Settings.


#include "GridCell.h"

// Sets default values
AGridCell::AGridCell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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


// Called when the game starts or when spawned
void AGridCell::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGridCell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

