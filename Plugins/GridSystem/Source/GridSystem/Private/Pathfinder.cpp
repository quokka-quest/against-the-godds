// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"

///////////////////////////////////////////////////////////////////////////////////////////////////// Pathing functions

// finds a path between the start and the target
bool PathFinder::FindPathBetweenCells(TArray<FPathInfo>& OutArray, FIntVector2 Start, FIntVector2 End, int Range)
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
	IsForRange = false;
	TotalMovement = (PathingRules.Contains(EPathingRules::RangeIsAvailableMovement))? Range : 100000;
	AttackRange = (!PathingRules.Contains(EPathingRules::RangeIsAvailableMovement))? Range : 100000;
	
	// clear maps
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();
	
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

// Finds every cell within the given range of the start
TSet<FIntVector2> PathFinder::FindAllCellsInRange(FIntVector2 Start, int Range)
{
	// early exit to avoid errors when attempting to path from a cell to itself
	if (Range == 0) { TSet<FIntVector2> Result; Result.Add(Start); return Result; }
	
	// set global variables
	StartCoord = Start;
	EndCoord = FIntVector2(10000, 10000); // set an impossible end to find so that all cells in the range are analysed
	IsForRange = true;
	TotalMovement = (PathingRules.Contains(EPathingRules::RangeIsAvailableMovement))? Range : 100000;
	AttackRange = (!PathingRules.Contains(EPathingRules::RangeIsAvailableMovement))? Range : 100000;
	
	// clear maps
	DiscoveredCells.Empty();
	AnalysedCells.Empty();
	CellMap.Empty();
	
	// discover the starting cell
	DiscoverCell(Start, Start, PathingData.CurrentRotation);

	// core A* analysis loop in this function
	TArray<FNewCellInfo> Results;
	PerformAnalysis(Results);

	return AnalysedCells;
}

///////////////////////////////////////////////////////////////////////////////////////////////////// Discover and Analysis (core A* loop)

// PARAMETERS
// OutArray: the array that will be filled with the path when its found
// RETURN
// returns true if a path was found and false otherwise
bool PathFinder::PerformAnalysis(TArray<FNewCellInfo>& OutArray)
{
	bool FoundPath = false;
	while (DiscoveredCells.Num() > 0 && !FoundPath)
	{
		FNewCellInfo CellInfo = GetNextCellToAnalyse();
		DiscoveredCells.Remove(CellInfo.Coord);
		AnalysedCells.Add(CellInfo.Coord);

		TArray<FNeighbourInfo> ValidNeighbours = GetValidNeighbours(CellInfo.Coord);
		if (ValidNeighbours.Num() == 0) continue;
	
		for (FNeighbourInfo Neighbour : ValidNeighbours)
		{
			// if the neighbour cell has never been discovered then discover it and move on
			if (!CellMap.Contains(Neighbour.Coord))
			{
				DiscoverCell(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
				if (Neighbour.Coord == EndCoord) { FoundPath = true; break; }
				continue;
			}

			int CurrentMovementCostFromStart = CellMap[Neighbour.Coord].MovementCostFromStart;
			int NewCostFromStart = CellInfo.MovementCostFromStart + CellMap[Neighbour.Coord].MovementCost;
			if (NewCostFromStart >= CurrentMovementCostFromStart) continue;

			CellMap.Remove(Neighbour.Coord);
			DiscoverCell(Neighbour.Coord, CellInfo.Coord, Neighbour.Direction);
			if (Neighbour.Coord == EndCoord) { FoundPath = true; break; }
		}
	}

	if (!FoundPath) return false;

	// Get the path by reversing through all the 'PrevCellCoord' values until the start is reached
	FIntVector2 PrevCoords = EndCoord;
	while (PrevCoords != StartCoord)
	{
		OutArray.Add(CellMap[PrevCoords]);
		PrevCoords = CellMap[PrevCoords].PrevCellCoord;
	}
	
	return true;
}

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
	CellInfo.MovementCostFromStart = (CellCoord == StartCoord)? 0: CellMap[PreviousCell].MovementCostFromStart + CellInfo.MovementCost;
	CellInfo.AbsDistFromTarget = CalculateMinCostBetweenCells(CellCoord, EndCoord);
	CellInfo.AbsDistFromStart = CalculateMinCostBetweenCells(StartCoord, CellCoord);
	CellInfo.PenaltyFromStart = (CellCoord == StartCoord)? 0 : CellMap[PreviousCell].PenaltyFromStart;
	CellInfo.PenaltyFromStart += GetPenaltyOfCoord(CellCoord);
	
	CellInfo.NewRotation = Direction;
	CellInfo.PrevRotation = (CellCoord == StartCoord)? Direction : CellMap[PreviousCell].NewRotation;

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

			// RULE: Exclude Occupied Cells
			// if this rule is to be followed then check for occupancy
			// excludes self occupancy
			if (PathingRules.Contains(EPathingRules::ExcludeOccupiedCells) && GridCells[Coord]->IsOccupied && GridCells[Coord]->OccupyingActor != PathingData.Actor) return;
		}
	}
	
	CellMap.Add(CellCoord, CellInfo);
	DiscoveredCells.Add(CellInfo.Coord);
}


///////////////////////////////////////////////////////////////////////////////////////////////////// Helper functions

// this function selects the next cell to be analysed
// priority: Lowest hazard penalty from the start, then shortest distance to the target
FNewCellInfo PathFinder::GetNextCellToAnalyse()
{
	FNewCellInfo CheapestCell = FNewCellInfo();
	int ShortestDist = 100000;
	int SmallestPenalty = 100000;

	if (IsForRange) return CellMap[DiscoveredCells[0]];

	for (FIntVector2 Coord : DiscoveredCells)
	{
		FNewCellInfo CellInfo = CellMap[Coord];
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

int PathFinder::CalculateMinCostBetweenCells(FIntVector2 Start, FIntVector2 End)
{
	return abs(Start.X - End.X) + abs(Start.Y - End.Y);
}

int PathFinder::GetPenaltyOfCoord(FIntVector2 Coord)
{
	int penalty = 0;
	
	// RULE: Try Path Around Hazards
	if (PathingRules.Contains(EPathingRules::TryPathAroundHazards)) penalty += (GridCells[Coord]->IsHazard)? 1 : 0;

	return penalty;
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
		EPatternRotation dir = CellMap[Coord].NewRotation;
		FIntVector2 Offset = (dir == R90)? FIntVector2(1, 0) : (dir == R0)? FIntVector2(0, 1) : (dir == R270)? FIntVector2(-1, 0) : FIntVector2(0, -1);
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