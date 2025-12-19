// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerTool.h"

void AGridManagerTool::BeginPlay()
{
	Super::BeginPlay();

	OutlineActor = GetWorld()->SpawnActor<AGridOutlineActor>(AGridOutlineActor::StaticClass());
	OutlineActor->SetMaterial(GridMaterial);

	float height = 10.2f;
	TMap<FVector, FVector> Edges = GenerateFullGridOutline(height);
	float LineWidth = 10.0f;
	OutlineActor->BuildOutlineMesh(Edges, LineWidth);
}


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

TMap<FVector, FVector> AGridManagerTool::GenerateFullGridOutline(float height)
{
	// find all the perimeter cells by checking if neighbours exist (a cell with all 4 neighbours is not on the perimeter)
	TArray<FOutlineCellInfo> PerimeterCells;
	for (auto Cell : GridCells)
	{
		FOutlineCellInfo CellInfo;
		CellInfo.CellCoord = Cell.Key;
		int NeighbourCount = 0;
		if (GridCells.Contains(Cell.Key + FIntVector2(1,0))) { NeighbourCount++; CellInfo.HasPosXNeighbour = true; }
		if (GridCells.Contains(Cell.Key + FIntVector2(-1,0))) { NeighbourCount++; CellInfo.HasNegXNeighbour = true; }
		if (GridCells.Contains(Cell.Key + FIntVector2(0,1))) { NeighbourCount++; CellInfo.HasPosYNeighbour = true; }
		if (GridCells.Contains(Cell.Key + FIntVector2(0,-1))) { NeighbourCount++; CellInfo.HasNegYNeighbour = true; }
		if (NeighbourCount < 4) PerimeterCells.Add(CellInfo);
	}

	// construct the map for start and end points of the grid highlight around each cell individually
	// NOTE: 'start' to 'end' must have a consistent rotation around the cell (clockwise or anti-clockwise)
	// The rotation means each 'End' will also be a 'Start' for the adjacent cell's outline which is important for the logic in the while loop below
	TMap<FVector, FVector> FullStartEndPoints;
	float HalfCellSizeX = GridCellSizeX * 0.5f;
	float HalfCellSizeY = GridCellSizeY * 0.5f;
	for (FOutlineCellInfo Cell : PerimeterCells)
	{
		if (!Cell.HasPosXNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, HalfCellSizeY, height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, -HalfCellSizeY, height);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasNegXNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, -HalfCellSizeY, height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, HalfCellSizeY, height);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasPosYNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, HalfCellSizeY, height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, HalfCellSizeY, height);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasNegYNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, -HalfCellSizeY, height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, -HalfCellSizeY, height);
			FullStartEndPoints.Add(Start, End);
		}
	}
	
	// construct a compressed map of the grid outline (adjacent cells with an outline in the same direction will be compressed to one outline)
	TMap<FVector, FVector> CompressedStartEndMap;
	TArray<FVector> StartPointArray;
	FullStartEndPoints.GenerateKeyArray(StartPointArray);
	while (StartPointArray.Num() > 0)
	{
		FVector Start = StartPointArray[0];
		FVector End = FullStartEndPoints[Start];
		FVector Dir = (End-Start).GetSafeNormal();
		StartPointArray.Remove(Start);

		FVector NextStart = End;
		FVector NextEnd = FullStartEndPoints[NextStart];
		FVector NextDir = (NextEnd-NextStart).GetSafeNormal();

		while (NextDir == Dir)
		{
			StartPointArray.Remove(NextStart);
			End = NextEnd;

			NextStart = End;
			NextEnd = FullStartEndPoints[NextStart];
			NextDir = (NextEnd-NextStart).GetSafeNormal();
		}
		CompressedStartEndMap.Add(Start, End);
	}

	return CompressedStartEndMap;
}
