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

	bool RequiresRotation;
	TEnumAsByte<EPatternRotation> NewRotation;
	TEnumAsByte<EPatternRotation> PrevRotation;

	bool operator==(const FCellInfo& Other) const
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
	PathFinder(TMap<FIntVector2, AGridCellBase*>& InGridCells, FPathingData& InPathingData):GridCells(InGridCells),PathingData(InPathingData){};

	TArray<FPathInfo> FindPath(FIntVector2 Start, FIntVector2 End, bool AvoidOccupiedCells = true);
	TArray<FIntVector2> FindMoveableCellsInRange(FIntVector2 Start, int AvailableMovement,  bool AvoidOccupiedCells = true);
	TArray<FIntVector2> FindAttackableCellsInRange(FIntVector2 Start, int Range, TArray<TEnumAsByte<EAttackRules>>& Rules);
	TArray<FPathInfo> FindPathToPointInRangeOfTarget(FIntVector2 Start, FIntVector2 End, int Range, TArray<TEnumAsByte<EAttackRules>>& Rules, bool AvoidOccupiedCells = true);

protected:
	TMap<FIntVector2, AGridCellBase*> GridCells;

	TArray<TEnumAsByte<EAttackRules>> AttackRules;

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
	void DiscoverCellForMovement(FIntVector2 CellCoord, FIntVector2 PreviousCell, TEnumAsByte<EPatternRotation> Direction);
	void DiscoverCellForAttack(FIntVector2 CellCoord, FIntVector2 PreviousCell, TEnumAsByte<EPatternRotation> Direction);

	// Analyse functions
	bool AnalyseNextCellForPathing();
	void AnalyseNextCellForMovement();
	void AnalyseNextCellForAttack();

	// helper functions

	int CalculateMinCostBetweenCells(FIntVector2 Start, FIntVector2 End);

	FCellInfo PullCheapestMoveCostCellFromDiscoveredCells();

	void MoveCellFromDiscoveredToAnalysed(FCellInfo Cell);

	bool IsCellAlreadyDiscovered(FIntVector2 CellCoord);

	TArray<FNeighbourInfo> GetValidNeighboursForMovement(FIntVector2 CellCoord);

	TArray<FNeighbourInfo> GetValidNeighboursForAttack(FIntVector2 CellCoord);

	FCellInfo PullNextAnalysableCell();

	TArray<FIntVector2> GetPerimeterCells(TArray<FIntVector2>& CellCoords);

	bool GetCellInArrayClosestToTarget(TArray<FIntVector2>& Cells, FIntVector2 Target, FIntVector2& OutCoord);

	bool CheckRotationSweep(FIntVector2 Coord);

	EPatternRotation GetDirectionBetweenTwoCells(FIntVector2 FromCoord, FIntVector2 ToCoord);

	bool CheckCoordIsValidNeighborForAttack(FIntVector2 Coord, FNeighbourInfo Neighbor);
};
