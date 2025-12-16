// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridData.h"
#include "GridCellBase.h"

struct FCellInfo
{
	FIntVector2 Coord;
	int EntryCost;
	int CostFromStart;
	int MinCostToTarget;
	FIntVector2 PrevCellCoord;

	bool operator==(const FCellInfo& Other) const
	{
		return (Coord == Other.Coord);
	}
};

class PathFinder
{
public:
	PathFinder(TMap<FIntVector2, AGridCellBase*>& InGridCells, FPathingData& InPathingData):GridCells(InGridCells),PathingData(InPathingData){};

	TArray<FIntVector2> FindPath(FIntVector2 Start, FIntVector2 End, bool AvoidOccupiedCells = true);
	TArray<FIntVector2> FindMoveableCellsInRange(FIntVector2 Start, int AvailableMovement,  bool AvoidOccupiedCells = true);
	TArray<FIntVector2> FindAttackableCellsInRange(FIntVector2 Start, int Range);
	TArray<FIntVector2> FindPathToPointInRangeOfTarget(FIntVector2 Start, FIntVector2 End, int Range, bool AvoidOccupiedCells = true);

protected:
	TMap<FIntVector2, AGridCellBase*> GridCells;

	FIntVector2 StartCoord = FIntVector2(0,0);
	FIntVector2 EndCoord = FIntVector2(0,0);

	TMap<FIntVector2, FCellInfo> CellMap;
	TArray<FCellInfo> DiscoveredCells;
	TArray<FCellInfo> AnalysedCells;

	int TotalMovement = 0;
	int AttackRange = 0;
	bool AvoidOccupied = true;
	FPathingData PathingData;

	// Discover functions
	void DiscoverCellForMovement(FIntVector2 CellCoord, FIntVector2 PreviousCell);
	void DiscoverCellForAttack(FIntVector2 CellCoord, FIntVector2 PreviousCell);

	// Analyse functions
	bool AnalyseNextCellForPathing();
	void AnalyseNextCellForMovement();
	void AnalyseNextCellForAttack();

	// helper functions

	int CalculateMinCostBetweenCells(FIntVector2 Start, FIntVector2 End);

	FCellInfo PullCheapestMoveCostCellFromDiscoveredCells();

	void MoveCellFromDiscoveredToAnalysed(FCellInfo Cell);

	bool IsCellAlreadyDiscovered(FIntVector2 CellCoord);

	TArray<FIntVector2> GetValidNeighbours(FIntVector2 CellCoord);

	FCellInfo PullNextAnalysableCell();

	TArray<FIntVector2> GetPerimeterCells(TArray<FIntVector2>& CellCoords);

	bool GetCellInArrayClosestToTarget(TArray<FIntVector2>& Cells, FIntVector2 Target, FIntVector2& OutCoord);
};
