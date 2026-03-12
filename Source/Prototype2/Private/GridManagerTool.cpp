// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerTool.h"
#include "GridOutlineGenerator.h"

void AGridManagerTool::BeginPlay()
{
	Super::BeginPlay();

	OutlineActor = GetWorld()->SpawnActor<AGridOutlineActor>(AGridOutlineActor::StaticClass());
	OutlineActor->SetMaterial(GridMaterial);
	AreaOutlineActor = GetWorld()->SpawnActor<AGridOutlineActor>(AGridOutlineActor::StaticClass());
	AreaOutlineActor->SetMaterial(AreaMaterial);
	PathAndAttackOutlineActor = GetWorld()->SpawnActor<AGridOutlineActor>(AGridOutlineActor::StaticClass());
	PathAndAttackOutlineActor->SetMaterial(PathMaterial);
	HighlightOutlineActor = GetWorld()->SpawnActor<AGridOutlineActor>(AGridOutlineActor::StaticClass());
	HighlightOutlineActor->SetMaterial(HighlightMaterial);

	float height = 0.0f;
	GridOutlineGenerator(OutlineActor).GenerateFullGridOutline(GridCells, height, GridCellSizeX, GridCellSizeY);

	FGridData HighlightData = FGridData();
	GridOutlineGenerator(HighlightOutlineActor).GenerateOutlineFromGridData(HighlightData, GridCellSizeX, GridCellSizeY);
}

void AGridManagerTool::ResetHighlights()
{
	OutlineActor->SetVisibility(true);
	AreaOutlineActor->SetVisibility(false);
	PathAndAttackOutlineActor->SetVisibility(false);
}

void AGridManagerTool::SetHighlightRotation(float Rotation)
{
	FRotator rot = FRotator(0.0f, Rotation, 0.0f);
	HighlightOutlineActor->SetActorRotation(rot);
}

void AGridManagerTool::DisplayWalkableCells(FIntVector2 Start, int AvailableMovement, FPathingData PathData)
{
	if (AvailableMovement <= 0) return;
	TArray<FIntVector2> WalkableCells = GetWalkableCells(Start, AvailableMovement, PathData);
	if (WalkableCells.Num() == 0) return;

	TArray<FIntVector2> DisplayCells;
	TArray<FIntVector2> SizeOffsets = PathData.RotationSweep.GetSelectedCellOffsets();
	for (int i = 0; i < WalkableCells.Num(); i++)
	{
		// the Pathfinder handles all validity checks so every cell passed back to here should be valid to walk on
		GridCells[WalkableCells[i]]->IsWalkable = true;

		for (FIntVector2 Offset : SizeOffsets)
		{
			FIntVector2 OffsetCoord = WalkableCells[i] + Offset;
			if (!GridCells.Contains(OffsetCoord)) {UE_LOG(LogTemp, Error, TEXT("GridManagerTool.cpp->DisplayWalkableCells: tried to display a cell that doesnt exist")); continue;}
			if (DisplayCells.Contains(OffsetCoord)) continue;
			DisplayCells.Add(OffsetCoord);
		}
	}
	
	
	GridOutlineGenerator(AreaOutlineActor).GenerateOutlineFromCoordArray(DisplayCells, Start, GridCellSizeX, GridCellSizeY);
	AreaOutlineActor->SetActorLocation(GridCells[Start]->GetActorLocation());
	AreaOutlineActor->SetVisibility(true);
	OutlineActor->SetVisibility(false);
}

TArray<FPathInfo> AGridManagerTool::DisplayCellPath(FIntVector2 StartCoord, FIntVector2 EndCoord, int AvailableMovement, FPathingData PathData)
{
	TArray<FPathInfo> Path = GetPathBetweenCoords(StartCoord, EndCoord, AvailableMovement, PathData);
	TArray<FIntVector2> DisplayCells;
	
	TArray<FIntVector2> SizeOffsets = PathData.RotationSweep.GetSelectedCellOffsets();
	for (FPathInfo Cell : Path)
	{
		for (FIntVector2 Offset : SizeOffsets)
		{
			FIntVector2 OffsetCoord = Cell.CoordToMoveTo + Offset;
			if (!GridCells.Contains(OffsetCoord)) { UE_LOG(LogTemp, Error, TEXT("GridManagerTool.cpp->DisplayCellPath: tried to display a cell that doesnt exist")); continue; }
			DisplayCells.Add(OffsetCoord);
		}
	}

	// add the start coord for display since 'GetPathBetweenCoords' does not contain the start coord
	for (FIntVector2 Offset : SizeOffsets)
	{
		FIntVector2 OffsetCoord = StartCoord + Offset;
		if (!GridCells.Contains(OffsetCoord)) { UE_LOG(LogTemp, Error, TEXT("GridManagerTool.cpp->DisplayCellPath: tried to display a cell that doesnt exist")); continue; }
		DisplayCells.Add(OffsetCoord);
	}

	GridOutlineGenerator(PathAndAttackOutlineActor).GenerateOutlineFromCoordArray(DisplayCells, StartCoord, GridCellSizeX, GridCellSizeY);
	PathAndAttackOutlineActor->SetActorLocation(GridCells[StartCoord]->GetActorLocation());
	PathAndAttackOutlineActor->SetVisibility(true);
	AreaOutlineActor->SetVisibility(false);

	return Path;
}

void AGridManagerTool::DisplayCellsInAttackRange(FIntVector2 Start, int Range, FPathingData PathData, TArray<TEnumAsByte<EAttackRules>>& Rules)
{
	if (Range < 0) return;
	TArray<FIntVector2> AttackableCells = GetCellsInAttackRange(Start, Range, PathData, Rules);
	if (AttackableCells.Num() == 0) return;

	TArray<FIntVector2> SizeOffsets = PathData.RotationSweep.GetSelectedCellOffsets();
	TArray<FIntVector2> DisplayCells;
	for (int i = 0; i < AttackableCells.Num(); i++)
	{
		if (!Rules.Contains(EAttackRules::MustFitOnTargetCell)) {GridCells[AttackableCells[i]]->IsAttackable = true; DisplayCells.Add(AttackableCells[i]); continue;}

		if (!DoesPatternFitOnCell(AttackableCells[i], SizeOffsets, PathData)) continue;
		GridCells[AttackableCells[i]]->IsAttackable = true;
		
		for (FIntVector2 Offset : SizeOffsets)
		{
			FIntVector2 OffsetCoord = AttackableCells[i]+Offset;
			DisplayCells.Add(OffsetCoord);
		}
	}

	GridOutlineGenerator(AreaOutlineActor).GenerateOutlineFromCoordArray(DisplayCells, Start, GridCellSizeX, GridCellSizeY);
	AreaOutlineActor->SetActorLocation(GridCells[Start]->GetActorLocation());
	AreaOutlineActor->SetVisibility(true);
	OutlineActor->SetVisibility(false);
}

TArray<FIntVector2> AGridManagerTool::DisplayAttackPattern(FIntVector2 TargetCoord, FGridData Pattern, EPatternRotation Rotation, FPathingData PathData, TArray<TEnumAsByte<EAttackRules>>& Rules)
{
	TArray<FIntVector2> Cells;
	if (!Rules.Contains(EAttackRules::PatternIsPath)) Cells = GetCellsInAttackArea(TargetCoord, Pattern, Rotation, PathData);
	else
	{
		FIntVector2 StartCoord = Cast<AEntityBase>(PathData.Actor)->PositionCoord;
		TArray<FPathInfo> Path = DisplayCellPath(TargetCoord, StartCoord, 1000, PathData);

		Cells.Add(TargetCoord);
		Cells.Add(StartCoord);
		for (FPathInfo& PathInfo : Path)
		{
			if (Cells.Contains(PathInfo.CoordToMoveTo)) continue;
			Cells.Add(PathInfo.CoordToMoveTo);
		}
		
		PathAndAttackOutlineActor->SetActorLocation(GridCells[TargetCoord]->GetActorLocation());
		PathAndAttackOutlineActor->SetVisibility(true);
		AreaOutlineActor->SetVisibility(false);
		return Cells;
	}
		
	if (Cells.Num() == 0) return Cells;

	GridOutlineGenerator(PathAndAttackOutlineActor).GenerateOutlineFromCoordArray(Cells, TargetCoord, GridCellSizeX, GridCellSizeY);
	PathAndAttackOutlineActor->SetActorLocation(GridCells[TargetCoord]->GetActorLocation());
	PathAndAttackOutlineActor->SetVisibility(true);
	AreaOutlineActor->SetVisibility(false);
	
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
#if WITH_EDITOR
			Arrow->SetFolderPath(FName("CellManager/Arrows"));
#endif
			DirectionIndicators.Add(Arrow);
		}
	}
}

void AGridManagerTool::SetHighlightPosition(FIntVector2 CellCoord)
{
	HighlightOutlineActor->SetActorLocation(GridCells[CellCoord]->GetActorLocation());
}

void AGridManagerTool::SetHighlightVisibility(bool IsVisible)
{
	HighlightOutlineActor->SetVisibility(IsVisible);
}

void AGridManagerTool::ChangeHighlightMesh(FGridData& HighlightData)
{
	GridOutlineGenerator(HighlightOutlineActor).GenerateOutlineFromGridData(HighlightData, GridCellSizeX, GridCellSizeY);
}

void AGridManagerTool::ToggleAxisIndicator()
{
	if (AxisActorRef) {AxisActorRef->Destroy(); AxisActorRef = nullptr; return;}
	
	FTransform spawnTrans = FTransform(FRotator(), FVector(0,0,30.0f), FVector(1));
	AActor* Arrow = GetWorld()->SpawnActor(AxisIndicator);
	Arrow->SetActorTransform(spawnTrans);
#if WITH_EDITOR
	Arrow->SetFolderPath(FName("CellManager/Arrows"));
#endif
	AxisActorRef = Arrow;
}

bool AGridManagerTool::DoesPatternFitOnCell(FIntVector2 CellCoord, TArray<FIntVector2>& Offsets, FPathingData& PathData)
{
	for (FIntVector2 Offset : Offsets)
	{
		FIntVector2 Coord = CellCoord + Offset;
		if (!GridCells.Contains(Coord)) return false;
		if (GridCells[Coord]->IsOccupied && PathData.Actor != GridCells[Coord]->OccupyingActor) return false;
	}

	return true;
}


void AGridManagerTool::ReplaceGridCell(UWorld* World, FIntVector2 Coord)
{
	// get old cell's variables
	float ZOffset = GridCells[Coord]->ZOffset;
	bool CanWalkThroughPositiveX = GridCells[Coord]->BlockPositiveX;
	bool CanWalkThroughNegativeX = GridCells[Coord]->BlockNegativeX;
	bool CanWalkThroughPositiveY = GridCells[Coord]->BlockPositiveY;
	bool CanWalkThroughNegativeY = GridCells[Coord]->BlockNegativeY;
	bool IsHazard = GridCells[Coord]->IsHazard;
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
	NewCell->IsHazard = IsHazard;
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
