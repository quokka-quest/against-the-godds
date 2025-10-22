// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	TArray<AActor*> Cells;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridCell::StaticClass(), Cells);
	GridCells.Empty();

	for (AActor* Actor : Cells)
	{
		AGridCell* Cell = Cast<AGridCell>(Actor);
		if (Cell)
		{
			GridCells.Add(Cell->GridCellCoord, Cell);
		}
	}

	if (!DefaulMat) {UE_LOG(LogTemp, Warning, TEXT("Default Material could not be found")) return;}
	if (!TargetMat) {UE_LOG(LogTemp, Warning, TEXT("Target Material could not be found")) return;}
	
	for (auto& Cell : GridCells)
	{
		FIntVector Key = Cell.Key;
		AGridCell* value = Cell.Value;
		
		UStaticMeshComponent* CellMesh = value->FindComponentByClass<UStaticMeshComponent>();
		if (!CellMesh) {UE_LOG(LogTemp, Warning, TEXT("Cell Mesh could not be found")) return;}

		if (GridDisplayType == EEditorGridDisplayType::HazardTile && value->IsEnviroHazardTile)
		{
			CellMesh->SetMaterial(0, TargetMat);
		}
		else if (GridDisplayType == EEditorGridDisplayType::PlayerSpawnTile && value->IsPlayerSpawnTile)
		{
			CellMesh->SetMaterial(0, TargetMat);
		}
		else
		{
			CellMesh->SetMaterial(0, DefaulMat);
		}
		
	}
}


