// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerClass.h"
#include "Engine/World.h"
#include "PathFinder.h"
#include "GameFramework/Actor.h"

////////////////////////////////////////////////////////////////////////////// Call in editor functions
void AGridManagerClass::OnConstruction(const FTransform& Transform)
{
	// a terrible work-around for getting this to only run on start up to refresh stale pointers
	if (HasRunBefore) return;
	HasRunBefore = true;

	Modify();

	if (GridCells.Num() > 0)
	{
		for (auto Cell : GridCells)
		{
			if (!Cell.Value) UE_LOG(LogTemp, Warning, TEXT("Cell not found at Coord: %i, %i"), Cell.Key.X, Cell.Key.Y) 
		}
	}
}

void AGridManagerClass::PrintAllCellCoords()
{
	for (auto Cell : GridCells)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cell key: %i, %i"), Cell.Key.X, Cell.Key.Y);
		UE_LOG(LogTemp, Warning, TEXT("Cells Coord: %i, %i"), Cell.Value->CellCoordinate.X, Cell.Value->CellCoordinate.Y)
	}
}

void AGridManagerClass::RegenerateGrid()
{
#if WITH_EDITOR
	if (!GridCellActor) return;
	Modify();
	
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld) {UE_LOG(LogTemp, Error, TEXT("GridSystemTool->RegenerateGrid() failed to find world")) return;}

	TArray<FIntVector2> UsedCoords;
	for (FIntVector2 Coord : GridData.GetSelectedCellOffsets())
	{
		UsedCoords.Add(Coord);
		// if the cell map already contain this coordinate then try to replace the cell object with the same parameters
		if (GridCells.Contains(Coord))
		{
			// If the Cell object exists then replace it
			if (GridCells[Coord]) { ReplaceGridCell(EditorWorld, Coord); continue; }
			// otherwise remove the coord from the map (in case user deletes a cell object manually)
			GridCells.Remove(Coord);
		}

		AGridCellBase* Cell = EditorWorld->SpawnActor<AGridCellBase>(GridCellActor);
		Cell->SetActorLocation(FVector(Coord.X * GridCellSizeX, Coord.Y * GridCellSizeY, 0));
		Cell->Modify();
		Cell->CellCoordinate = Coord;
		GridCells.Add(Coord, Cell);
	}

	this->SetFolderPath(FName("CellManager"));
	// look through the map and if it contains a coordinate that was not in the selected array, destroy that cell
	TArray<FIntVector2> CoordsToRemove;
	for (auto Cell : GridCells)
	{
		if (UsedCoords.Contains(Cell.Key))
		{
			GridCells[Cell.Key]->SetFolderPath(FName("CellManager/Cells"));
			continue;
		} // add code to make grid cells child of the manager
		CoordsToRemove.Add(Cell.Key);
		if (GridCells[Cell.Key]) EditorWorld->DestroyActor(GridCells[Cell.Key]);
	}
	// separate for loop for map coord removal to prevent errors from modifying the Map while looping through it above
	for (FIntVector2 Coord : CoordsToRemove)
	{
		GridCells.Remove(Coord);
	}
#endif
}

////////////////////////////////////////////////////////////////////////////// Helper Functions
// Replaces a cell object, maintaining the parameters of the old cell object
void AGridManagerClass::ReplaceGridCell(UWorld* World, FIntVector2 Coord)
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
}

void AGridManagerClass::RotateOffsets(TArray<FIntVector2>& Offsets, EPatternRotation Rotation)
{
	if (Rotation == R90)
	{
		for (int i = 0; i < Offsets.Num(); i++)
		{
			int32 NewX = Offsets[i].Y;
			int32 NewY = -Offsets[i].X;
			Offsets[i] = FIntVector2(NewX, NewY);
		}
	}
	else if (Rotation == R180)
	{
		for (int i = 0; i < Offsets.Num(); i++)
		{
			int32 NewX = -Offsets[i].X;
			int32 NewY = -Offsets[i].Y;
			Offsets[i] = FIntVector2(NewX, NewY);
		}
	}
	else if (Rotation == R270)
	{
		for (int i = 0; i < Offsets.Num(); i++)
		{
			int32 NewX = -Offsets[i].Y;
			int32 NewY = Offsets[i].X;
			Offsets[i] = FIntVector2(NewX, NewY);
		}
	}
}


////////////////////////////////////////////////////////////////////////////// Pathfinding functions

void AGridManagerClass::ResetWalkableAndAttackableOnAllCells()
{
	for (auto Cell : GridCells)
	{
		Cell.Value->IsWalkable = false;
		Cell.Value->IsAttackable = false;
	}
}

TArray<FIntVector2> AGridManagerClass::GetWalkableCells(FIntVector2 StartCoord, int AvailableMovement, FPathingData PathingData)
{
	return PathFinder(GridCells, PathingData).FindMoveableCellsInRange(StartCoord, AvailableMovement);
}

TArray<FIntVector2> AGridManagerClass::GetCellsInAttackRange(FIntVector2 StartCoord, int Range, FPathingData PathingData, TArray<TEnumAsByte<EAttackRules>>& Rules)
{
	return PathFinder(GridCells, PathingData).FindAttackableCellsInRange(StartCoord, Range, Rules);
}

TArray<FIntVector2> AGridManagerClass::GetCellsInAttackArea(FIntVector2 Target, FGridData AttackPattern, EPatternRotation Rotation, FPathingData PathingData)
{
	TArray<FIntVector2> Offsets = AttackPattern.GetSelectedCellOffsets();
	RotateOffsets(Offsets, Rotation);

	TArray<FIntVector2> Results;
	for (int i = 0; i < Offsets.Num(); i++)
	{
		if (!GridCells.Contains(Target + Offsets[i])) continue;
		Results.Add(Target + Offsets[i]);
	}
	return Results;
}

TArray<FPathInfo> AGridManagerClass::GetPathToPointInRangeOfTarget(FIntVector2 Start, FIntVector2 End, int Range, FPathingData PathingData, TArray<TEnumAsByte<EAttackRules>>& Rules)
{
	return PathFinder(GridCells, PathingData).FindPathToPointInRangeOfTarget(Start, End, Range, Rules);
}

TArray<FPathInfo> AGridManagerClass::GetPathBetweenCoords(FIntVector2 Start, FIntVector2 End, FPathingData PathingData, bool AvoidOccupied)
{
	return PathFinder(GridCells, PathingData).FindPath(Start, End, AvoidOccupied);
}
