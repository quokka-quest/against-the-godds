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

void AGridManagerTool::DisplayWalkableCells(FIntVector2 Start, int AvailableMovement, FPathingData PathData)
{
	if (AvailableMovement <= 0) return;
	TArray<FIntVector2> WalkableCells = GetWalkableCells(Start, AvailableMovement, PathData);
	if (WalkableCells.Num() == 0) return;

	for (int i = 0; i < WalkableCells.Num(); i++)
	{
		GridCells[WalkableCells[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, HighlightedMat);
		GridCells[WalkableCells[i]]->IsWalkable = true;
	}
}

TArray<FPathInfo> AGridManagerTool::DisplayCellPath(FIntVector2 StartCoord, FIntVector2 EndCoord, FPathingData PathData)
{
	TArray<FPathInfo> Path = GetPathBetweenCoords(StartCoord, EndCoord, PathData);

	for (FPathInfo Cell : Path)
	{
		GridCells[Cell.CoordToMoveTo]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, PathMat);
	}
	
	return Path;
}

void AGridManagerTool::DisplayCellsInAttackRange(FIntVector2 Start, int Range, FPathingData PathData)
{
	if (Range < 0) return;
	TArray<FIntVector2> AttackableCells = GetCellsInAttackRange(Start, Range, PathData);
	if (AttackableCells.Num() == 0) return;

	for (int i = 0; i < AttackableCells.Num(); i++)
	{
		GridCells[AttackableCells[i]]->IsAttackable = true;
		GridCells[AttackableCells[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, HighlightedMat);
	}
}

TArray<FIntVector2> AGridManagerTool::DisplayAttackPattern(FIntVector2 TargetCoord, FGridData Pattern, EPatternRotation Rotation, FPathingData PathData)
{
	TArray<FIntVector2> Cells = GetCellsInAttackArea(TargetCoord, Pattern, Rotation, PathData);
	if (Cells.Num() == 0) return Cells;

	for (int i = 0; i < Cells.Num(); i++)
	{
		GridCells[Cells[i]]->FindComponentByClass<UStaticMeshComponent>()->SetMaterial(0, TargetMat);
	}

	return Cells;
}

TArray<FIntVector2> AGridManagerTool::GetPlayerSpawnCells()
{
	TArray<FIntVector2> PlayerSpawnCells;
	for (auto& Cell : GridCells)
	{
		FIntVector2 Coord = Cell.Key;
		AGridCellParent* CellRef = Cast<AGridCellParent>(Cell.Value);
		if (CellRef->IsPlayerSpawnCell) PlayerSpawnCells.Add(Coord);
	}
	return PlayerSpawnCells;
}

void AGridManagerTool::ToggleDirectionIndicators()
{
	for (AActor* Arrow : DirectionIndicators)
	{
		if (!Arrow) continue;
		Arrow->Destroy();
	}
	DisplayArrows = !DisplayArrows;
	if (!DisplayArrows) return;

	for (auto& Cell : GridCells)
	{
		AGridCellParent* CellRef = Cast<AGridCellParent>(Cell.Value);
		if ((CellRef->IsPlayerSpawnCell && GridDisplayType == PlayerSpawnTile) || (CellRef->IsEnemySpawnCell && GridDisplayType == EnemySpawnTile))
		{
			EPatternRotation temp = CellRef->SpawnedEntityRotation;
			FRotator spawnRot = FRotator(90, 0, (temp == R0)? 0: (temp == R90)? 90: (temp == R180)? 180: -90);
			FVector spawnPos = CellRef->GetActorLocation() + FVector(0, 0, 100);
			FTransform spawnTrans = FTransform(spawnRot, spawnPos, FVector(1));
			AActor* Arrow = GetWorld()->SpawnActor(ArrowIndicator);
			Arrow->SetActorTransform(spawnTrans);
			Arrow->SetFolderPath(FName("CellManager/Arrows"));
			DirectionIndicators.Add(Arrow);
		}
	}
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
	TEnumAsByte<EPatternRotation> EntityDirection = R0;
	
	if (Cell)
	{
		PlayerSpawn = Cell->IsPlayerSpawnCell;
		EnviroHazard = Cell->IsEnviroHazardCell;
		EnemySpawn = Cell->IsEnemySpawnCell;
		EnemyToSpawn = Cell->EnemyToSpawn;
		EntityDirection = Cell->SpawnedEntityRotation;
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
		NewerCell->SpawnedEntityRotation = EntityDirection;
	}
}