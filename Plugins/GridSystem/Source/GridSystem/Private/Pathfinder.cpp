// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"

///////////////////////////////////////////////////////////////////////////////////////////////////// Pathfinding Functions
TArray<FPathInfo> PathFinder::FindPath(FIntVector2 Start, FIntVector2 End, bool AvoidOccupiedCells)
{
	StartCoord = Start;
	EndCoord = End;
	TotalMovement = 1000;
	AvoidOccupied = AvoidOccupiedCells;
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();

	TArray<FIntVector2> ReversPathCoords;
	TArray<FPathInfo> Result;

	// discover the starting tile
	DiscoverCellForMovement(Start, Start, PathingData.CurrentRotation);

	int LoopCount = 0;
	while (DiscoveredCells.Num() > 0)
	{
		// safety to prevent an infinite loop crash
		LoopCount++;
		if (LoopCount > 50000) {UE_LOG(LogTemp, Error, TEXT("Pathfinder->FindPath error in pathfinding: infinite loop detected")) return Result;}

		// analyse the next tile and if the end cell has not been found then continue
		bool FoundEndPoint = AnalyseNextCellForPathing();
		if (!FoundEndPoint) continue;

		// if a path has been found then populate the result array and return it

		// retrieve the coordinates of the cells in the path (done from end to start)
		ReversPathCoords.Add(EndCoord);
		FIntVector2 PrevCoord = EndCoord;
		while (PrevCoord != StartCoord)
		{
			PrevCoord = CellMap[PrevCoord].PrevCellCoord;
			ReversPathCoords.Add(PrevCoord);
		}

		// establish the first movement
		FPathInfo FirstMovement;
		FirstMovement.StartingCoord = StartCoord;
		FirstMovement.CoordToMoveTo = ReversPathCoords[ReversPathCoords.Num()-2];
		FirstMovement.StartingRot = PathingData.CurrentRotation;
		FirstMovement.RotToChangeTo = GetDirectionBetweenTwoCells(FirstMovement.StartingCoord, FirstMovement.CoordToMoveTo);
		Result.Add(FirstMovement);
		int resultIndex = 0;
		// complete the rest of the path
		for (int i = ReversPathCoords.Num()-2; i > 0; i--)
		{
			FPathInfo NextMovement;
			NextMovement.StartingCoord = ReversPathCoords[i];
			NextMovement.CoordToMoveTo = ReversPathCoords[i-1];
			NextMovement.StartingRot = Result[resultIndex].RotToChangeTo;
			NextMovement.RotToChangeTo = GetDirectionBetweenTwoCells(NextMovement.StartingCoord, NextMovement.CoordToMoveTo);
			Result.Add(NextMovement);
			resultIndex++;
		}
		
		return Result;
	}

	return Result;
}

TArray<FIntVector2> PathFinder::FindMoveableCellsInRange(FIntVector2 Start, int AvailableMovement, bool AvoidOccupiedCells)
{
	TotalMovement = AvailableMovement;
	AvoidOccupied = AvoidOccupiedCells;
	StartCoord = Start;
	EndCoord = Start;
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();

	TArray<FIntVector2> Result;

	DiscoverCellForMovement(Start, Start, PathingData.CurrentRotation);

	while (DiscoveredCells.Num() > 0)
	{
		AnalyseNextCellForMovement();
	}

	for (FCellInfo Cell : AnalysedCells)
	{
		Result.Add(Cell.Coord);
	}
	
	return Result;
}

TArray<FIntVector2> PathFinder::FindAttackableCellsInRange(FIntVector2 Start, int Range, TArray<TEnumAsByte<EAttackRules>>& Rules)
{
	AttackRange = Range;
	StartCoord = Start;
	AttackRules = Rules;
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();

	TArray<FIntVector2> Result;

	DiscoverCellForAttack(Start, Start, PathingData.CurrentRotation);

	while (DiscoveredCells.Num() > 0)
	{
		AnalyseNextCellForAttack();
	}

	for (FCellInfo Cell : AnalysedCells)
	{
		Result.Add(Cell.Coord);
	}

	return Result;
}

TArray<FPathInfo> PathFinder::FindPathToPointInRangeOfTarget(FIntVector2 Start, FIntVector2 End, int Range, TArray<TEnumAsByte<EAttackRules>>& Rules, bool AvoidOccupiedCells)
{
	TArray<FIntVector2> CellsInAttackRange = FindAttackableCellsInRange(End, Range, Rules);
	TArray<FIntVector2> PerimeterCells = GetPerimeterCells(CellsInAttackRange);

	FIntVector2 ClosestTile = FIntVector2(0);
	if (!GetCellInArrayClosestToTarget(PerimeterCells, Start, ClosestTile)) return FindPath(Start, End);

	return FindPath(Start, ClosestTile);
}


///////////////////////////////////////////////////////////////////////////////////////////////////// Discover functions
// These functions Discover a cell, checking for movement/attack availability
// the cell coord to be discovered and the cell that it's connected to are passed in
// the 'Direction' parameter is the rotation that the entity will have upon entering the new cell
void PathFinder::DiscoverCellForMovement(FIntVector2 CellCoord, FIntVector2 PreviousCell, TEnumAsByte<EPatternRotation> Direction)
{
	FCellInfo CellInfo;
	CellInfo.Coord = CellCoord;
	CellInfo.EntryCost = GridCells[CellCoord]->MovementCost;
	CellInfo.MinCostToTarget = CalculateMinCostBetweenCells(CellCoord, EndCoord);
	CellInfo.PrevCellCoord = PreviousCell;
	CellInfo.NewRotation = Direction;
	CellInfo.PrevRotation = (CellCoord == StartCoord)? Direction : CellMap[PreviousCell].NewRotation;
	CellInfo.RequiresRotation = CellInfo.NewRotation != CellInfo.PrevRotation;

	// if the cell map has contents then the previous coord can be checked since it will have been added already
	// otherwise set it to 0 (makes the starting tile have an entry cost of 0)
	int CostFromStart = (!CellMap.IsEmpty())? CellMap[PreviousCell].CostFromStart + CellInfo.EntryCost: 0;
	CellInfo.CostFromStart = CostFromStart;

	if (CostFromStart > TotalMovement) return;
	if (CellCoord != StartCoord && CellCoord != EndCoord && AvoidOccupied && GridCells[CellCoord]->IsOccupied && GridCells[CellCoord]->OccupyingActor != PathingData.Actor) return;

	// check if entity can fit in the new area it will move to
	for (FIntVector2 Offset  : PathingData.ActorRotations[PathingData.CurrentRotation].GetSelectedCellOffsets())
	{
		FIntVector2 Coord = CellCoord + Offset;
		if (!GridCells.Contains(Coord)) return;
		if (GridCells[Coord]->IsOccupied && GridCells[Coord]->OccupyingActor != PathingData.Actor) return;
	}

	CellMap.Add(CellCoord, CellInfo);
	DiscoveredCells.Add(CellInfo);
}

void PathFinder::DiscoverCellForAttack(FIntVector2 CellCoord, FIntVector2 PreviousCell, TEnumAsByte<EPatternRotation> Direction)
{
	FCellInfo CellInfo;
	CellInfo.Coord = CellCoord;
	CellInfo.EntryCost = 1;
	CellInfo.MinCostToTarget = 0;
	CellInfo.PrevCellCoord = PreviousCell;
	CellInfo.NewRotation = Direction;

	int CostFromStart = (!CellMap.IsEmpty())? CellMap[PreviousCell].CostFromStart + CellInfo.EntryCost: 0;
	CellInfo.CostFromStart = CostFromStart;

	if (CostFromStart > AttackRange) return;

	CellMap.Add(CellCoord, CellInfo);
	DiscoveredCells.Add(CellInfo);
}


///////////////////////////////////////////////////////////////////////////////////////////////////// Analyse functions
bool PathFinder::AnalyseNextCellForPathing()
{
	FCellInfo CellInfo = PullCheapestMoveCostCellFromDiscoveredCells();
	MoveCellFromDiscoveredToAnalysed(CellInfo);

	TArray<FNeighbourInfo> ValidNeighbours = GetValidNeighboursForMovement(CellInfo.Coord);
	if (ValidNeighbours.Num() == 0) return false;
	
	for (FNeighbourInfo Neighbour : ValidNeighbours)
	{
		if (IsCellAlreadyDiscovered(Neighbour.Coord)) continue;

		DiscoverCellForMovement(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
		if (Neighbour.Coord == EndCoord) return true;
	}
	
	return false;
}

void PathFinder::AnalyseNextCellForMovement()
{
	FCellInfo CellInfo = PullNextAnalysableCell();
	TArray<FNeighbourInfo> ValidNeighbours = GetValidNeighboursForMovement(CellInfo.Coord);

	if (ValidNeighbours.Num() == 0) return;

	bool IsRotationAllowed = CheckRotationSweep(CellInfo.Coord);

	for (FNeighbourInfo Neighbour : ValidNeighbours)
	{
		if (IsCellAlreadyDiscovered(Neighbour.Coord)) continue;
		if (!IsRotationAllowed && Neighbour.Direction != CellInfo.NewRotation) continue;

		if (!CheckRotationSweep(Neighbour.Coord)) continue; // can not move onto a cell that can't be moved off of
		
		DiscoverCellForMovement(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
	}
}

void PathFinder::AnalyseNextCellForAttack()
{
	FCellInfo CellInfo = PullNextAnalysableCell();
	TArray<FNeighbourInfo> ValidNeighbours = GetValidNeighboursForAttack(CellInfo.Coord);

	if (ValidNeighbours.Num() == 0) return;

	for (FNeighbourInfo Neighbour : ValidNeighbours)
	{
		if (IsCellAlreadyDiscovered(Neighbour.Coord)) continue;

		DiscoverCellForAttack(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////// Helper functions

int PathFinder::CalculateMinCostBetweenCells(FIntVector2 Start, FIntVector2 End)
{
	return abs(Start.X - End.X) + abs(Start.Y - End.Y);
}

// get the cell in the discovered array that has the cheapest cost to the end target
FCellInfo PathFinder::PullCheapestMoveCostCellFromDiscoveredCells()
{
	FCellInfo CheapestCell = FCellInfo();
	int CheapestCost = 100000;

	for (FCellInfo Cell : DiscoveredCells)
	{
		if (Cell.MinCostToTarget >= CheapestCost) continue;
		CheapestCell = Cell;
		CheapestCost = Cell.MinCostToTarget;
	}

	return CheapestCell;
}

void PathFinder::MoveCellFromDiscoveredToAnalysed(FCellInfo Cell)
{
	DiscoveredCells.Remove(Cell);
	AnalysedCells.Add(Cell);
}

bool PathFinder::IsCellAlreadyDiscovered(FIntVector2 CellCoord)
{
	return CellMap.Contains(CellCoord);
}

// returns the valid neighbours for a given cell. Checks if a direction can be travelled through and if the coordinate exists
TArray<FNeighbourInfo> PathFinder::GetValidNeighboursForMovement(FIntVector2 CellCoord)
{
	TArray<FNeighbourInfo> ValidNeighbours;

	FNeighbourInfo PosXNeighbour = FNeighbourInfo(CellCoord + FIntVector2(1,0), R90);
	FNeighbourInfo PosYNeighbour = FNeighbourInfo(CellCoord + FIntVector2(0,1), R0);
	FNeighbourInfo NegXNeighbour = FNeighbourInfo(CellCoord + FIntVector2(-1,0), R270);
	FNeighbourInfo NegYNeighbour = FNeighbourInfo(CellCoord + FIntVector2(0,-1), R180);
	
	if (GridCells.Contains(PosXNeighbour.Coord)
		&& !GridCells[CellCoord]->BlockPositiveX
		&& !GridCells[PosXNeighbour.Coord]->BlockNegativeX) ValidNeighbours.Add(PosXNeighbour);

	if (GridCells.Contains(NegXNeighbour.Coord)
		&& !GridCells[CellCoord]->BlockNegativeX
		&& !GridCells[NegXNeighbour.Coord]->BlockPositiveX) ValidNeighbours.Add(NegXNeighbour);

	if (GridCells.Contains(PosYNeighbour.Coord)
		&& !GridCells[CellCoord]->BlockPositiveY
		&& !GridCells[PosYNeighbour.Coord]->BlockNegativeY) ValidNeighbours.Add(PosYNeighbour);

	if (GridCells.Contains(NegYNeighbour.Coord)
		&& !GridCells[CellCoord]->BlockNegativeY
		&& !GridCells[NegYNeighbour.Coord]->BlockPositiveY) ValidNeighbours.Add(NegYNeighbour);
	
	return ValidNeighbours;
}

TArray<FNeighbourInfo> PathFinder::GetValidNeighboursForAttack(FIntVector2 CellCoord)
{
	TArray<FNeighbourInfo> ValidNeighbours;

	FNeighbourInfo PosXNeighbour = FNeighbourInfo(CellCoord + FIntVector2(1,0), R90);
	FNeighbourInfo PosYNeighbour = FNeighbourInfo(CellCoord + FIntVector2(0,1), R0);
	FNeighbourInfo NegXNeighbour = FNeighbourInfo(CellCoord + FIntVector2(-1,0), R270);
	FNeighbourInfo NegYNeighbour = FNeighbourInfo(CellCoord + FIntVector2(0,-1), R180);

	bool ObeyTraversal = AttackRules.Contains(EAttackRules::ObeyTraversalRules);
	bool StraightLine = AttackRules.Contains(EAttackRules::StraightLineOnly);

	if (GridCells.Contains(PosXNeighbour.Coord) && CheckCoordIsValidNeighborForAttack(CellCoord, PosXNeighbour)) ValidNeighbours.Add(PosXNeighbour);
	if (GridCells.Contains(PosYNeighbour.Coord) && CheckCoordIsValidNeighborForAttack(CellCoord, PosYNeighbour)) ValidNeighbours.Add(PosYNeighbour);
	if (GridCells.Contains(NegXNeighbour.Coord) && CheckCoordIsValidNeighborForAttack(CellCoord, NegXNeighbour)) ValidNeighbours.Add(NegXNeighbour);
	if (GridCells.Contains(NegYNeighbour.Coord) && CheckCoordIsValidNeighborForAttack(CellCoord, NegYNeighbour)) ValidNeighbours.Add(NegYNeighbour);

	return ValidNeighbours;
}

bool PathFinder::CheckCoordIsValidNeighborForAttack(FIntVector2 Coord, FNeighbourInfo Neighbor)
{
	if (Coord == StartCoord) return true; // return true if checking for the start coord
	if (AttackRules.IsEmpty()) return true; // return true is there are no rules to follow (ToDo: need to implement line of sight checks)
	if (AttackRules.Num() == 1 && AttackRules.Contains(EAttackRules::IgnoreLineOfSight)) return true; // see the 'ToDo' above

	bool ObeyTraversal = AttackRules.Contains(EAttackRules::ObeyTraversalRules);
	bool StraightLine = AttackRules.Contains(EAttackRules::StraightLineOnly);

	bool IsStraight = (CellMap[Coord].NewRotation == Neighbor.Direction);
	
	bool ObeysTravel = false;
	if (Neighbor.Direction == R90) {ObeysTravel = (!GridCells[Coord]->BlockPositiveX && !GridCells[Neighbor.Coord]->BlockNegativeX);}
	else if (Neighbor.Direction == R270) {ObeysTravel = (!GridCells[Coord]->BlockNegativeX && !GridCells[Neighbor.Coord]->BlockPositiveX);}
	else if (Neighbor.Direction == R0) {ObeysTravel = (!GridCells[Coord]->BlockPositiveY && !GridCells[Neighbor.Coord]->BlockNegativeY);}
	else {ObeysTravel = (!GridCells[Coord]->BlockNegativeY && !GridCells[Neighbor.Coord]->BlockPositiveY);}

	if (ObeyTraversal && StraightLine) { return (IsStraight && ObeysTravel); }
	if (ObeyTraversal && ObeysTravel) { return true; }
	if (StraightLine && IsStraight) { return true; }

	return false;
}


FCellInfo PathFinder::PullNextAnalysableCell()
{
	FCellInfo Cell = DiscoveredCells[0];
	MoveCellFromDiscoveredToAnalysed(Cell);
	return Cell;
}

TArray<FIntVector2> PathFinder::GetPerimeterCells(TArray<FIntVector2>& CellCoords)
{
	TArray<FIntVector2> result;

	for (int i = 0; i < CellCoords.Num(); i++)
	{
		int NumOfNeighbours = 0;
		if (CellCoords.Contains(CellCoords[i] + FIntVector2(1,0))) NumOfNeighbours++;
		if (CellCoords.Contains(CellCoords[i] + FIntVector2(-1,0))) NumOfNeighbours++;
		if (CellCoords.Contains(CellCoords[i] + FIntVector2(0,1))) NumOfNeighbours++;
		if (CellCoords.Contains(CellCoords[i] + FIntVector2(0,-1))) NumOfNeighbours++;
		if (NumOfNeighbours < 4) result.Add(CellCoords[i]);
	}
	
	return result;
}

bool PathFinder::GetCellInArrayClosestToTarget(TArray<FIntVector2>& Cells, FIntVector2 Target, FIntVector2& OutCoord)
{
	bool Result = false;
	int ShortestDist = 10000;

	for (int i = 0; i < Cells.Num(); i++)
	{
		int Dist = CalculateMinCostBetweenCells(Cells[i], Target);
		if (Dist >= ShortestDist) continue;
		if (GridCells[Cells[i]]->IsOccupied) continue;
		if (!CheckRotationSweep(Cells[i])) continue;
		ShortestDist = Dist;
		OutCoord = Cells[i];
		Result = true;
	}
	
	return Result;
}

// used to check if rotation of the entity is possible (stops large entities being able to rotate through walls)
bool PathFinder::CheckRotationSweep(FIntVector2 Coord)
{
	for (FIntVector2 Offset : PathingData.RotationSweep.GetSelectedCellOffsets())
	{
		if (!GridCells.Contains(Coord+Offset)) return false;
		if (GridCells[Coord+Offset]->IsOccupied && GridCells[Coord+Offset]->OccupyingActor != PathingData.Actor) return false;
	}
	return true;
}

EPatternRotation PathFinder::GetDirectionBetweenTwoCells(FIntVector2 FromCoord, FIntVector2 ToCoord)
{
	EPatternRotation Result = R0;
	FIntVector2 Offset = ToCoord - FromCoord;

	if (Offset == FIntVector2(1,0)) Result = R90;
	if (Offset == FIntVector2(0,1)) Result = R0;
	if (Offset == FIntVector2(-1,0)) Result = R270;
	if (Offset == FIntVector2(0,-1)) Result = R180;
	
	return Result;
}
