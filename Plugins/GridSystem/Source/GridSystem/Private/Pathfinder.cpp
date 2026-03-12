// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"

///////////////////////////////////////////////////////////////////////////////////////////////////// Reworking functions
bool PathFinder::FindPathBetweenCells(TArray<FPathInfo>& OutArray, FIntVector2 Start, FIntVector2 End, int Range, TArray<TEnumAsByte<EPathingRules>>& Rules)
{
	// early exit to avoid errors when attempting to path from a cell to itself
	OutArray.Empty();
	if (Start == End)
	{
		FPathInfo path;
		path.StartingCoord = Start;
		path.CoordToMoveTo = Start;
		path.StartingRot = PathingData.CurrentRotation;
		path.RotToChangeTo = PathingData.CurrentRotation;
		path.HazardPenaltyFromStart = 0;
		OutArray.Add(path);
		return true;
	}
	
	// set global variables
	StartCoord = Start;
	EndCoord = End;
	TotalMovement = (Rules.Contains(EPathingRules::RangeIsAvailableMovement))? Range : 100000;
	AttackRange = (!Rules.Contains(EPathingRules::RangeIsAvailableMovement))? Range : 100000;
	PathingRules = Rules;
	
	// clear maps
	NewDiscoveredCells.Empty();
	NewAnalysedCells.Empty();
	NewCellMap.Empty();
	
	// discover the starting cell
	DiscoverCell(Start, Start, PathingData.CurrentRotation);

	// core A* analysis loop in this function
	TArray<FNewCellInfo> Result;
	if (!PerformAnalysis(Result)) return false;

	for (int i = Result.Num() - 1; i >= 0; i--)
	{
		FPathInfo PathSegment = FPathInfo();
		PathSegment.StartingCoord = Result[i].PrevCellCoord;
		PathSegment.CoordToMoveTo = Result[i].Coord;
		PathSegment.StartingRot = Result[i].PrevRotation;
		PathSegment.RotToChangeTo = Result[i].NewRotation;
		PathSegment.HazardPenaltyFromStart = Result[i].PenaltyFromStart;
		OutArray.Add(PathSegment);
	}
	
	return true;
}

// Absolute discovery with new rule system
// PARAMETERS
// CellCoord: The coordinate being discovered
// PreviousCell: the coordinate of the cell from which the path moved onto this one
// Direction: The direction of travel onto the new cell
void PathFinder::DiscoverCell(FIntVector2 CellCoord, FIntVector2 PreviousCell, TEnumAsByte<EPatternRotation> Direction)
{
	FNewCellInfo CellInfo;
	CellInfo.Coord = CellCoord;
	CellInfo.PrevCellCoord = PreviousCell;
	
	CellInfo.MovementCost = (CellCoord == StartCoord)? 0: GridCells[CellCoord]->MovementCost;
	CellInfo.MovementCostFromStart = (CellCoord == StartCoord)? 0: NewCellMap[PreviousCell].MovementCostFromStart + CellInfo.MovementCost;
	CellInfo.AbsDistFromTarget = CalculateMinCostBetweenCells(CellCoord, EndCoord);
	CellInfo.AbsDistFromStart = CalculateMinCostBetweenCells(StartCoord, CellCoord);
	CellInfo.PenaltyFromStart = (CellCoord == StartCoord)? 0 : NewCellMap[PreviousCell].PenaltyFromStart;
	CellInfo.PenaltyFromStart += GetPenaltyOfCoord(CellCoord);
	
	CellInfo.NewRotation = Direction;
	CellInfo.PrevRotation = (CellCoord == StartCoord)? Direction : NewCellMap[PreviousCell].NewRotation;

	// do not discover if movement limit is exceeded
	if (CellInfo.MovementCostFromStart > TotalMovement) return;

	// do not discover if the cell is out of range (range ignores movement cost)
	if (CellInfo.AbsDistFromStart > AttackRange) return;

	// RULE: Exclude Occupied Cells
	// if the rule is active then do not discover cells that contain an object
	// the starting cell is excluded from this rule applying
	if (PathingRules.Contains(EPathingRules::ExcludeOccupiedCells) && CellCoord != StartCoord && GridCells[CellCoord]->IsOccupied && GridCells[CellCoord]->OccupyingActor != PathingData.Actor) return;

	// RULE: Exclude Hazard Cells
	// if this rule is active then do not discover cells marked as hazards
	// the starting cell is excluded from this rule applying
	if (PathingRules.Contains(EPathingRules::ExcludeHazardCells) && CellCoord != StartCoord && GridCells[CellCoord]->IsHazard) return;

	// RULE: Must Fit On Target Cell
	// check if entity can fit in the new area
	// best used for movement or movement based abilities
	if (PathingRules.Contains(EPathingRules::MustFitOnTarget))
	{
		for (FIntVector2 Offset  : PathingData.ActorRotations[PathingData.CurrentRotation].GetSelectedCellOffsets())
		{
			FIntVector2 Coord = CellCoord + Offset;
			if (!GridCells.Contains(Coord)) return;
			if (GridCells[Coord]->IsOccupied && GridCells[Coord]->OccupyingActor != PathingData.Actor && AvoidOccupied) return;
		}
	}

	NewCellMap.Add(CellCoord, CellInfo);
	NewDiscoveredCells.Add(CellInfo.Coord);
}

// Absolute analysis with new rule system
// PARAMETERS
// OutArray: the array that will be filled with the path when its found
// RETURN
// returns true if a path was found and false otherwise
bool PathFinder::PerformAnalysis(TArray<FNewCellInfo>& OutArray)
{
	bool FoundPath = false;
	while (NewDiscoveredCells.Num() > 0 && !FoundPath)
	{
		FNewCellInfo CellInfo = GetNextCellToAnalyse();
		NewDiscoveredCells.Remove(CellInfo.Coord);
		NewAnalysedCells.Add(CellInfo.Coord);

		TArray<FNeighbourInfo> ValidNeighbours = GetValidNeighbours(CellInfo.Coord);
		if (ValidNeighbours.Num() == 0) return false;
	
		for (FNeighbourInfo Neighbour : ValidNeighbours)
		{
			// if the neighbour cell has never been discovered then discover it and move on
			if (!NewCellMap.Contains(Neighbour.Coord))
			{
				DiscoverCell(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
				if (Neighbour.Coord == EndCoord) { FoundPath = true; break; }
				continue;
			}

			int CurrentMovementCostFromStart = NewCellMap[Neighbour.Coord].MovementCostFromStart;
			int NewCostFromStart = CellInfo.MovementCostFromStart + NewCellMap[Neighbour.Coord].MovementCost;
			if (NewCostFromStart >= CurrentMovementCostFromStart) continue;

			NewCellMap.Remove(Neighbour.Coord);
			DiscoverCell(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
			if (Neighbour.Coord == EndCoord) { FoundPath = true; break; }
		}
	}

	if (!FoundPath) return false;

	// Get the path by reversing through all the 'PrevCellCoord' values until the start is reached
	FIntVector2 PrevCoords = EndCoord;
	while (PrevCoords != StartCoord)
	{
		OutArray.Add(NewCellMap[PrevCoords]);
		PrevCoords = NewCellMap[PrevCoords].PrevCellCoord;
	}
	
	return true;
}

// this function selects the next cell to be analysed
// priority: Lowest hazard penalty from the start, then shortest distance to the target
FNewCellInfo PathFinder::GetNextCellToAnalyse()
{
	FNewCellInfo CheapestCell = FNewCellInfo();
	int ShortestDist = 100000;
	int SmallestPenalty = 100000;

	for (FIntVector2 Coord : NewDiscoveredCells)
	{
		FNewCellInfo CellInfo = NewCellMap[Coord];
		if (CellInfo.PenaltyFromStart > SmallestPenalty) continue;

		// a smaller penalty has absolute priority over a smaller distance to the target
		if (CellInfo.PenaltyFromStart < SmallestPenalty)
		{
			SmallestPenalty = CellInfo.PenaltyFromStart;
			ShortestDist = CellInfo.AbsDistFromTarget;
			CheapestCell = CellInfo;
			continue;
		}

		// if the penalty is the same then just check the distance to the target is smaller
		if (CellInfo.AbsDistFromTarget >= ShortestDist) continue;
		CheapestCell = CellInfo;
		ShortestDist = CellInfo.AbsDistFromTarget;
	}

	return CheapestCell;
}

// This function returns a hashset containing all the neighbours valid for pathing
// use by the analysis function to know which cells to 'discover'
TArray<FNeighbourInfo> PathFinder::GetValidNeighbours(FIntVector2 Coord)
{
	TArray<FNeighbourInfo> ValidNeighbours;
	TArray<FNeighbourInfo> Neighbours;

	// RULE: Straight Line Only
	// if this rule is active, then only check the neighbour that is in the straight line direction from the previous cell
	// except if it's the start coordinate, in which case add all of them because they all start their own unique straight line
	if (PathingRules.Contains(EPathingRules::StraightLine) && Coord != StartCoord)
	{
		EPatternRotation dir = NewCellMap[Coord].NewRotation;
		FIntVector2 Offset = FIntVector2((dir == R90)? 1 : (dir == R270)? -1 : 0, (dir == R0)? 1 : (dir == R180)? -1 : 0);
		Neighbours.Add(FNeighbourInfo(Coord + Offset, dir));
	}
	else
	{
		Neighbours.Add(FNeighbourInfo(Coord + FIntVector2(1,0), R90));
		Neighbours.Add(FNeighbourInfo(Coord + FIntVector2(0,1), R0));
		Neighbours.Add(FNeighbourInfo(Coord + FIntVector2(-1,0), R270));
		Neighbours.Add(FNeighbourInfo(Coord + FIntVector2(0,-1), R180));
	}

	for (FNeighbourInfo Neighbour : Neighbours)
	{
		// if the neighbour exists and can be moved to then add it to the valid array
		if (GridCells.Contains(Neighbour.Coord) && IsCoordAValidNeighbour(Coord, Neighbour)) ValidNeighbours.Add(Neighbour);
	}

	return ValidNeighbours;
}

// this function checks for the 'blocked' boolean in the Cells
// blocked is meant as an absolute rejection of pathing in the specified direction so there is no rule to enable/disable it
// blocked is best used where walls are supposed to be between cells, so this will stop walking / shooting through walls
bool PathFinder::IsCoordAValidNeighbour(FIntVector2 Coord, FNeighbourInfo& Neighbour)
{
	if (Neighbour.Direction == R90) { return (!GridCells[Coord]->BlockPositiveX && !GridCells[Neighbour.Coord]->BlockNegativeX); }
	if (Neighbour.Direction == R270) { return (!GridCells[Coord]->BlockNegativeX && !GridCells[Neighbour.Coord]->BlockPositiveX); }
	if (Neighbour.Direction == R0) { return (!GridCells[Coord]->BlockPositiveY && !GridCells[Neighbour.Coord]->BlockNegativeY); }
	return (!GridCells[Coord]->BlockNegativeY && !GridCells[Neighbour.Coord]->BlockPositiveY);
}

int PathFinder::GetPenaltyOfCoord(FIntVector2 Coord)
{
	int penalty = 0;
	
	// RULE: Try Path Around Hazards
	if (PathingRules.Contains(EPathingRules::TryPathAroundHazards)) penalty += (GridCells[Coord]->IsHazard)? 1 : 0;

	return penalty;
}


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
		FirstMovement.HazardPenaltyFromStart = 0;
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
			NextMovement.HazardPenaltyFromStart = 0;
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
		if (GridCells[Coord]->IsOccupied && GridCells[Coord]->OccupyingActor != PathingData.Actor && AvoidOccupied) return;
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

		if (AttackRules.Contains(EAttackRules::ObeyTraversalRules) && !CheckRotationSweep(Neighbour.Coord)) continue;

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
	if (AttackRules.IsEmpty()) return true; // return true if there are no rules to follow (ToDo: need to implement line of sight checks)
	if (AttackRules.Num() == 1 && AttackRules.Contains(EAttackRules::IgnoreLineOfSight)) return true; // see the 'ToDo' above

	bool ObeyTraversal = AttackRules.Contains(EAttackRules::ObeyTraversalRules);
	bool StraightLine = AttackRules.Contains(EAttackRules::StraightLineOnly);

	bool IsStraight = (CellMap[Coord].NewRotation == Neighbor.Direction);
	
	bool ObeysTravel;
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
