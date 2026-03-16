// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridData.h"
#include "GridCellBase.h"

struct FNewCellInfo
{
	FIntVector2 Coord;
	FIntVector2 PrevCellCoord;
	
	int MovementCost;
	int MovementCostFromStart;
	int AbsDistFromTarget;
	int PenaltyFromStart;
	int AbsDistFromStart;

	TEnumAsByte<EPatternRotation> NewRotation;
	TEnumAsByte<EPatternRotation> PrevRotation;

	bool operator==(const FNewCellInfo& Other) const
	{
		return (Coord == Other.Coord);
	}
};

struct FNeighbourInfo
{
	FIntVector2 Coord;
	TEnumAsByte<EPatternRotation> Direction;

	FNeighbourInfo()
	{
		Coord = FIntVector2(0,0);
		Direction = R0;
	}
	FNeighbourInfo(FIntVector2 coord, TEnumAsByte<EPatternRotation> direction)
	{
		Coord = coord;
		Direction = direction;
	}
};

class PathFinder
{
public:
	PathFinder(TMap<FIntVector2, AGridCellBase*>& InGridCells, FPathingData& InPathingData, TArray<TEnumAsByte<EPathingRules>> InPathingRules):GridCells(InGridCells),PathingData(InPathingData),PathingRules(InPathingRules){};

	bool FindPathBetweenCells(TArray<FPathInfo>& OutArray, FIntVector2 Start, FIntVector2 End, int Range);
	TSet<FIntVector2> FindAllCellsInRange(FIntVector2 Start, int Range);

protected:
	// constructor variables
	TMap<FIntVector2, AGridCellBase*> GridCells; // map of all existing cells
	TArray<TEnumAsByte<EPathingRules>> PathingRules; // rules to follow
	FPathingData PathingData; // pathing data of actor the path is for

	// maps and sets
	TMap<FIntVector2, FNewCellInfo> CellMap;
	TArray<FIntVector2> DiscoveredCells;
	TSet<FIntVector2> AnalysedCells;

	// global variables
	FIntVector2 StartCoord = FIntVector2(0,0);
	FIntVector2 EndCoord = FIntVector2(0,0);
	int TotalMovement = 0;
	int AttackRange = 0;
	bool IsForRange = false;

	// discover and analyse
	void DiscoverCell(FIntVector2 CellCoord, FIntVector2 PreviousCell, TEnumAsByte<EPatternRotation> Direction);
	bool PerformAnalysis(TArray<FNewCellInfo>& OutArray);

	// helper functions
	int CalculateMinCostBetweenCells(FIntVector2 Start, FIntVector2 End);
	FNewCellInfo GetNextCellToAnalyse();
	TArray<FNeighbourInfo> GetValidNeighbours(FIntVector2 Coord);
	bool IsCoordAValidNeighbour(FIntVector2 Coord, FNeighbourInfo& Neighbour);
	int GetPenaltyOfCoord(FIntVector2 Coord);
};
