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

	InitialiseGridManagement();
	
}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// called in the editor when variables are changed. Useful for displaying the right information
void AGridManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InitialiseGridManagement();
	ToggleTileVisibility(GridDisplayType);
}

void AGridManager::InitialiseGridManagement()
{
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

	if (!DefaultMat) UE_LOG(LogTemp, Warning, TEXT("Default Material could not be found"))
	if (!TargetMat) UE_LOG(LogTemp, Warning, TEXT("Target Material could not be found"))
	if (!HighlightedMat) UE_LOG(LogTemp, Warning, TEXT("Highlight Material could not be found"))
}


void AGridManager::ToggleTileVisibility(EEditorGridDisplayType DisplayType)
{
	for (auto& Cell : GridCells)
	{
		FIntVector Key = Cell.Key;
		AGridCell* value = Cell.Value;
		
		UStaticMeshComponent* CellMesh = value->FindComponentByClass<UStaticMeshComponent>();
		if (!CellMesh) {UE_LOG(LogTemp, Warning, TEXT("Cell Mesh could not be found")) return;}

		if (DisplayType == EEditorGridDisplayType::HazardTile && value->IsEnviroHazardTile)
		{
			CellMesh->SetMaterial(0, HighlightedMat);
		}
		else if (DisplayType == EEditorGridDisplayType::PlayerSpawnTile && value->IsPlayerSpawnTile)
		{
			CellMesh->SetMaterial(0, HighlightedMat);
		}
		else
		{
			CellMesh->SetMaterial(0, DefaultMat);
		}

		//UE_LOG(LogTemp, Warning, TEXT("toggled a tiles display material"));
	}
}

void AGridManager::ChangeTilesMaterial(AGridCell* Tile, ETileMaterial Material)
{
	UStaticMeshComponent* CellMesh = Tile->FindComponentByClass<UStaticMeshComponent>();
	CellMesh->SetMaterial(0, (Material == ETileMaterial::Target)? TargetMat : (Material == ETileMaterial::Highlighted)? HighlightedMat : DefaultMat);
}



