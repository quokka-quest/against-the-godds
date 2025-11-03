// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PathFinder = nullptr;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	InitialiseGridManagement();
	
}

// called in the editor when variables are changed. Useful for displaying the right information
void AGridManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InitialiseGridManagement();
	ChangeAllTilesDisplay(GridDisplayType);
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

	PathFinder = GetWorld()->SpawnActor<APathFinder>();

	if (!DefaultMat) UE_LOG(LogTemp, Warning, TEXT("Default Material could not be found"))
	if (!TargetMat) UE_LOG(LogTemp, Warning, TEXT("Target Material could not be found"))
	if (!HighlightedMat) UE_LOG(LogTemp, Warning, TEXT("Highlight Material could not be found"))
	if (!PathMat) UE_LOG(LogTemp, Warning, TEXT("Path Material could not be found"))
}


void AGridManager::ChangeAllTilesDisplay(EEditorGridDisplayType DisplayType)
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
		else if (DisplayType == EEditorGridDisplayType::EnemySpawnTile && value->IsEnemySpawnTile)
		{
			CellMesh->SetMaterial(0, HighlightedMat);
		}
		else
		{
			CellMesh->SetMaterial(0, DefaultMat);
		}
	}
}

void AGridManager::ChangeTilesMaterial(AGridCell* Tile, ETileMaterial Material)
{
	UStaticMeshComponent* CellMesh = Tile->FindComponentByClass<UStaticMeshComponent>();
	CellMesh->SetMaterial(0, (Material == ETileMaterial::Target)? TargetMat : (Material == ETileMaterial::Highlighted)? HighlightedMat : DefaultMat);
}

void AGridManager::DisplayWalkableTiles(FIntVector CurrentCellCoord, int AvailableMovement)
{
	if (AvailableMovement <= 0) return;

	TArray<FIntVector> WalkableCoords = PathFinder->FindMoveableTiles(CurrentCellCoord, AvailableMovement);
	if (WalkableCoords.Num() == 0) return;

	for (FIntVector WalkableCoord : WalkableCoords)
	{
		GridCells[WalkableCoord]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, HighlightedMat);
		GridCells[WalkableCoord]->isWalkable = true;
	}
}

void AGridManager::ResetTilesWalkAndAttackBooleans()
{
	for (auto& Cell : GridCells)
	{
		Cell.Value->isWalkable = false;
		Cell.Value->isAttackable = false;
		Cell.Value->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, DefaultMat);
	}
}

TArray<FIntVector> AGridManager::DisplayTilePath(FIntVector StartCoord, FIntVector EndCoord)
{
	TArray<FIntVector> Path = PathFinder->FindPath(StartCoord, EndCoord);
	if (Path.IsEmpty()) { UE_LOG(LogTemp, Error, TEXT("Path could not be found")) return Path; }

	for (int i = 0; i < Path.Num(); i++)
	{
		GridCells[Path[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, PathMat);
	}

	return Path;
}

void AGridManager::DisplayTilesInAttackRange(FIntVector CurrentCellCoord, int Range)
{
	if (Range < 0) return;

	TArray<FIntVector> WalkableCoords = PathFinder->FindAttackableTiles(CurrentCellCoord, Range);
	if (WalkableCoords.Num() == 0) return;

	for (FIntVector WalkableCoord : WalkableCoords)
	{
		GridCells[WalkableCoord]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, HighlightedMat);
		GridCells[WalkableCoord]->isAttackable = true;
	}
}





