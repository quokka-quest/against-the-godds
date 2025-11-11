// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridCell.h"
#include "GameFramework/Actor.h"

struct FTileInfo
{
	FIntVector Coord;
	int EntryCost;
	int CostFromStart;
	int MinCostToTarget;
	FIntVector PreviousTile;

	bool operator==(const FTileInfo& Other) const
	{
		return (Coord == Other.Coord);
	}
};

// NOTE: Currently this pathfinding ignores the idea of Z levels
// functionality for different heights will need to be added later
class PathFinder
{
public:
	PathFinder(TMap<FIntVector, AGridCell*>& InGridCells):GridCells(InGridCells){};

	void Initialise(TMap<FIntVector, AGridCell*>& InGridCells);
	
	TArray<FIntVector> FindPath(FIntVector Start, FIntVector End);

	TArray<FIntVector> FindMoveableTiles(FIntVector Start, int AvailableMovement);

	TArray<FIntVector> FindAttackableTiles(FIntVector Start, int Range);

	TArray<FIntVector> FindPathForEnemy(FIntVector Start, FIntVector End);

private:
	TMap<FIntVector, AGridCell*>& GridCells;
	
	FIntVector StartCoord = FIntVector(0);
	FIntVector EndCoord = FIntVector(0);

	TMap<FIntVector, FTileInfo> TileMap;
	TArray<FTileInfo> DiscoveredTiles;
	TArray<FTileInfo> AnalysedTiles;

	int TotalMovement = 0;
	int AttackRange = 0;

	int CalulateMinCostBetweenTiles(FIntVector Start, FIntVector End);

	void DiscoverTileForMovement(FIntVector TileCoord, FIntVector PreviousTile);

	void DiscoverTileForAttack(FIntVector TileCoord, FIntVector PreviousTile);

	FTileInfo PullCheapestTileFromDiscoveredArray();

	bool IsTileAlreadyDiscovered(FIntVector TileCoord);

	bool AnalyseNextTile();

	void MoveTileFromDiscoveredToAnalysed(FTileInfo Tile);

	FTileInfo GetNextTileFromDiscoverableArray();

	void AnalyseTileForMovementAvailability();

	void AnalyseTileForAttackAvailability();

	bool AnalyseTileForEnemyMovement();

	void DiscoverTileForEnemyMovement(FIntVector TileCoord, FIntVector PreviousTile);

	TArray<FIntVector> NeighbourOffsets =
	{
		FIntVector(1,0,0),
		FIntVector(-1,0,0),
		FIntVector(0,1,0),
		FIntVector(0,-1,0)
	};

};
