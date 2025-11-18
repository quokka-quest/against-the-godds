// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"

///////////////////////////////////////////////////////////////////////////////////////////////////// Pathfinding Functions
TArray<FIntVector2> PathFinder::FindPath(FIntVector2 Start, FIntVector2 End, bool AvoidOccupiedCells)
{
	StartCoord = Start;
	EndCoord = End;
	TotalMovement = 1000;
	AvoidOccupied = AvoidOccupiedCells;
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();

	TArray<FIntVector2> Result;

	// discover the starting tile
	DiscoverCellForMovement(Start, Start);

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
		Result.Add(CellMap[EndCoord].Coord);
		FIntVector2 PrevCoord = EndCoord;

		while (PrevCoord != StartCoord)
		{
			PrevCoord = CellMap[PrevCoord].PrevCellCoord;
			Result.Add(PrevCoord);
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
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();

	TArray<FIntVector2> Result;

	DiscoverCellForMovement(Start, Start);

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

TArray<FIntVector2> PathFinder::FindAttackableCellsInRange(FIntVector2 Start, int Range)
{
	AttackRange = Range;
	StartCoord = Start;
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();

	TArray<FIntVector2> Result;

	DiscoverCellForAttack(Start, Start);

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

TArray<FIntVector2> PathFinder::FindPathToPointInRangeOfTarget(FIntVector2 Start, FIntVector2 End, int Range, bool AvoidOccupiedCells)
{
	TArray<FIntVector2> CellsInAttackRange = FindAttackableCellsInRange(End, Range);
	TArray<FIntVector2> PerimeterCells = GetPerimeterCells(CellsInAttackRange);

	FIntVector2 ClosestTile = FIntVector2(0);
	if (!GetCellInArrayClosestToTarget(PerimeterCells, Start, ClosestTile)) return FindPath(Start, End);

	return FindPath(Start, ClosestTile);
}


///////////////////////////////////////////////////////////////////////////////////////////////////// Discover functions
// These functions Discover a cell, checking for movement/attack availability
// the cell coord to be discovered and the cell that it's connected to are passed in

void PathFinder::DiscoverCellForMovement(FIntVector2 CellCoord, FIntVector2 PreviousCell)
{
	FCellInfo CellInfo;
	CellInfo.Coord = CellCoord;
	CellInfo.EntryCost = GridCells[CellCoord]->MovementCost;
	CellInfo.MinCostToTarget = CalculateMinCostBetweenCells(CellCoord, EndCoord);
	CellInfo.PrevCellCoord = PreviousCell;

	// if the cell map has contents then the previous coord can be checked since it will have been added already
	// otherwise set it to 0 (makes the starting tile have an entry cost of 0)
	int CostFromStart = (!CellMap.IsEmpty())? CellMap[PreviousCell].CostFromStart + CellInfo.EntryCost: 0;
	CellInfo.CostFromStart = CostFromStart;

	if (CostFromStart > TotalMovement) return;
	if (CellCoord != StartCoord && CellCoord != EndCoord && AvoidOccupied && GridCells[CellCoord]->IsOccupied) return;
	CellMap.Add(CellCoord, CellInfo);
	DiscoveredCells.Add(CellInfo);
}

void PathFinder::DiscoverCellForAttack(FIntVector2 CellCoord, FIntVector2 PreviousCell)
{
	FCellInfo CellInfo;
	CellInfo.Coord = CellCoord;
	CellInfo.EntryCost = 1;
	CellInfo.MinCostToTarget = 0;
	CellInfo.PrevCellCoord = PreviousCell;

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

	TArray<FIntVector2> ValidNeighbours = GetValidNeighbours(CellInfo.Coord);
	if (ValidNeighbours.Num() == 0) return false;
	
	for (FIntVector2 Neighbour : ValidNeighbours)
	{
		if (IsCellAlreadyDiscovered(Neighbour)) continue;

		DiscoverCellForMovement(Neighbour, CellInfo.Coord);
		if (Neighbour == EndCoord) return true;
	}
	
	return false;
}

void PathFinder::AnalyseNextCellForMovement()
{
	FCellInfo CellInfo = PullNextAnalysableCell();
	TArray<FIntVector2> ValidNeighbours = GetValidNeighbours(CellInfo.Coord);

	if (ValidNeighbours.Num() == 0) return;

	for (FIntVector2 Neighbour : ValidNeighbours)
	{
		if (IsCellAlreadyDiscovered(Neighbour)) continue;

		DiscoverCellForMovement(Neighbour, CellInfo.Coord);
	}
}

void PathFinder::AnalyseNextCellForAttack()
{
	FCellInfo CellInfo = PullNextAnalysableCell();
	TArray<FIntVector2> ValidNeighbours = GetValidNeighbours(CellInfo.Coord);

	if (ValidNeighbours.Num() == 0) return;

	for (FIntVector2 Neighbour : ValidNeighbours)
	{
		if (IsCellAlreadyDiscovered(Neighbour)) continue;

		DiscoverCellForAttack(Neighbour, CellInfo.Coord);
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
TArray<FIntVector2> PathFinder::GetValidNeighbours(FIntVector2 CellCoord)
{
	TArray<FIntVector2> ValidNeighbours;

	FIntVector2 PosXNeighbour = CellCoord + FIntVector2(1,0);
	FIntVector2 PosYNeighbour = CellCoord + FIntVector2(0,1);
	FIntVector2 NegXNeighbour = CellCoord + FIntVector2(-1,0);
	FIntVector2 NegYNeighbour = CellCoord + FIntVector2(0,-1);
	
	if (GridCells.Contains(PosXNeighbour)
		&& !GridCells[CellCoord]->BlockPositiveX
		&& !GridCells[PosXNeighbour]->BlockPositiveX) ValidNeighbours.Add(PosXNeighbour);

	if (GridCells.Contains(NegXNeighbour)
		&& !GridCells[CellCoord]->BlockNegativeX
		&& !GridCells[NegXNeighbour]->BlockNegativeX) ValidNeighbours.Add(NegXNeighbour);

	if (GridCells.Contains(PosYNeighbour)
		&& !GridCells[CellCoord]->BlockPositiveY
		&& !GridCells[PosYNeighbour]->BlockPositiveY) ValidNeighbours.Add(PosYNeighbour);

	if (GridCells.Contains(NegYNeighbour)
		&& !GridCells[CellCoord]->BlockNegativeY
		&& !GridCells[NegYNeighbour]->BlockNegativeY) ValidNeighbours.Add(NegYNeighbour);
	
	return ValidNeighbours;
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
		ShortestDist = Dist;
		OutCoord = Cells[i];
		Result = true;
	}
	
	return Result;
}
