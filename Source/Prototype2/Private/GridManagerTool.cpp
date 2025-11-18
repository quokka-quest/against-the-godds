// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerTool.h"

void AGridManagerTool::UpdateDisplay()
{
	ChangeAllTilesDisplay(GridDisplayType);
}

void AGridManagerTool::ChangeAllTilesDisplay(EEditorGridDisplayType DisplayType)
{
	for (auto& Cell : GridCells)
	{
		AGridCellParent* value = Cast<AGridCellParent>(Cell.Value);
		if (!value) {UE_LOG(LogTemp, Warning, TEXT("GridManagerTool->ChangeAllTilesDisplay: Cell failed to cast to AGridCellParent")) return;}
		
		UStaticMeshComponent* CellMesh = value->FindComponentByClass<UStaticMeshComponent>();
		if (!CellMesh) {UE_LOG(LogTemp, Warning, TEXT("Cell Mesh could not be found")) return;}

		if (DisplayType == EEditorGridDisplayType::HazardTile && value->IsEnviroHazardCell)
		{
			CellMesh->SetMaterial(0, HighlightedMat);
		}
		else if (DisplayType == EEditorGridDisplayType::PlayerSpawnTile && value->IsPlayerSpawnCell)
		{
			CellMesh->SetMaterial(0, HighlightedMat);
		}
		else if (DisplayType == EEditorGridDisplayType::EnemySpawnTile && value->IsEnemySpawnCell)
		{
			CellMesh->SetMaterial(0, HighlightedMat);
		}
		else
		{
			CellMesh->SetMaterial(0, DefaultMat);
		}
	}
}

void AGridManagerTool::ChangeCellsMaterial(AGridCellParent* Tile, ETileMaterial Material)
{
	UStaticMeshComponent* CellMesh = Tile->FindComponentByClass<UStaticMeshComponent>();
	CellMesh->SetMaterial(0, (Material == ETileMaterial::Target)? TargetMat : (Material == ETileMaterial::Highlighted)? HighlightedMat : DefaultMat);
}

void AGridManagerTool::DisplayWalkableCells(FIntVector2 Start, int AvailableMovement)
{
	if (AvailableMovement <= 0) return;
	TArray<FIntVector2> WalkableCells = GetWalkableCells(Start, AvailableMovement);
	if (WalkableCells.Num() == 0) return;

	for (int i = 0; i < WalkableCells.Num(); i++)
	{
		GridCells[WalkableCells[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, HighlightedMat);
		GridCells[WalkableCells[i]]->IsWalkable = true;
	}
}

TArray<FIntVector2> AGridManagerTool::DisplayCellPath(FIntVector2 StartCoord, FIntVector2 EndCoord)
{
	TArray<FIntVector2> Path = GetPathBetweenCoords(StartCoord, EndCoord);

	for (FIntVector2 Cell : Path)
	{
		GridCells[Cell]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, PathMat);
	}
	
	return Path;
}

void AGridManagerTool::DisplayCellsInAttackRange(FIntVector2 Start, int Range)
{
	if (Range < 0) return;
	TArray<FIntVector2> AttackableCells = GetCellsInAttackRange(Start, Range);
	if (AttackableCells.Num() == 0) return;

	for (int i = 0; i < AttackableCells.Num(); i++)
	{
		GridCells[AttackableCells[i]]->IsAttackable = true;
		GridCells[AttackableCells[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, HighlightedMat);
	}
}

TArray<FIntVector2> AGridManagerTool::DisplayAttackPattern(FIntVector2 TargetCoord, FGridData Pattern, EPatternRotation Rotation)
{
	TArray<FIntVector2> Cells = GetCellsInAttackArea(TargetCoord, Pattern, Rotation);
	if (Cells.Num() == 0) return Cells;

	for (int i = 0; i < Cells.Num(); i++)
	{
		GridCells[Cells[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, TargetMat);
	}

	return Cells;
}

void AGridManagerTool::ReplaceGridCell(UWorld* World, FIntVector2 Coord)
{
	// get old cell's variables
	float ZOffset = GridCells[Coord]->ZOffset;
	bool CanWalkThroughPositiveX = GridCells[Coord]->BlockPositiveX;
	bool CanWalkThroughNegativeX = GridCells[Coord]->BlockNegativeX;
	bool CanWalkThroughPositiveY = GridCells[Coord]->BlockPositiveY;
	bool CanWalkThroughNegativeY = GridCells[Coord]->BlockNegativeY;
	bool IsWalkable = GridCells[Coord]->IsWalkable;
	int MovementCost = GridCells[Coord]->MovementCost;
	bool IsAttackable = GridCells[Coord]->IsAttackable;

	// Extra variables
	AGridCellParent* Cell = Cast<AGridCellParent>(GridCells[Coord]);
	bool PlayerSpawn = false;
	bool EnviroHazard = false;
	bool EnemySpawn = false;
	TSubclassOf<AEnemyEntity> EnemyToSpawn = nullptr;
	
	if (Cell)
	{
		PlayerSpawn = Cell->IsPlayerSpawnCell;
		EnviroHazard = Cell->IsEnviroHazardCell;
		EnemySpawn = Cell->IsEnemySpawnCell;
		EnemyToSpawn = Cell->EnemyToSpawn;
		
	}

	// destroy old cell
	World->DestroyActor(GridCells[Coord]);

	// spawn the new cell
	FTransform NewCellTransform;
	NewCellTransform.SetLocation(FVector(Coord.X * GridCellSizeX, Coord.Y * GridCellSizeY, 0));

	FActorSpawnParameters SpawnParams;
#if WITH_EDITOR
	SpawnParams.bHideFromSceneOutliner = false;
#endif
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transactional;
	
	AGridCellBase* NewCell = World->SpawnActor<AGridCellBase>(GridCellActor, NewCellTransform, SpawnParams);
	NewCell->SetActorLocation(FVector(NewCell->GetActorLocation().X, NewCell->GetActorLocation().Y, ZOffset));

	NewCell->Modify();
	Modify();
	NewCell->CellCoordinate = Coord;
	GridCells[Coord] = NewCell;
	
	// set new cell's variables
	NewCell->ZOffset = ZOffset;
	NewCell->BlockPositiveX = CanWalkThroughPositiveX;
	NewCell->BlockNegativeX = CanWalkThroughNegativeX;
	NewCell->BlockPositiveY = CanWalkThroughPositiveY;
	NewCell->BlockNegativeY = CanWalkThroughNegativeY;
	NewCell->IsWalkable = IsWalkable;
	NewCell->MovementCost = MovementCost;
	NewCell->IsAttackable = IsAttackable;
	NewCell->CellCoordinate = Coord;

	// Extra variables
	AGridCellParent* NewerCell = Cast<AGridCellParent>(NewCell);
	
	if (NewerCell)
	{
		NewerCell->Modify();
		NewerCell->IsPlayerSpawnCell = PlayerSpawn;
		NewerCell->IsEnviroHazardCell = EnviroHazard;
		NewerCell->IsEnemySpawnCell = EnemySpawn;
		NewerCell->EnemyToSpawn = EnemyToSpawn;
		
	}
}