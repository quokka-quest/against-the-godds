// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathFinder.generated.h"

USTRUCT()
struct FTileInfo
{
	GENERATED_BODY()

	FIntVector Coord;
	int EntryCost;
	int CostFromStart;
	int MinCostToTarget;
	FIntVector PreviousTile;
};

// NOTE: Currently this pathfinding ignores the idea of Z levels
// functionality for different heights will need to be added later
UCLASS()
class PROTOTYPE2_API APathFinder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APathFinder();

	UFUNCTION()
	TArray<FIntVector> FindPath(FIntVector Start, FIntVector End);

private:
	FIntVector StartCoord;
	FIntVector EndCoord;

	TMap<FIntVector, FTileInfo> TileMap;
	TArray<FTileInfo> DiscoveredTiles;
	TArray<FTileInfo> AnalysedTiles;

	int CalulateMinCostBetweenTiles(FIntVector Start, FIntVector End);

	void DiscoverTile(FIntVector TileCoord, FIntVector PreviousTile);

	FTileInfo PullCheapestTileFromDiscoveredArray();

	bool IsTileAlreadyDiscovered(FIntVector TileCoord);

	bool AnalyseNextTile();

	void MoveTileFromDiscoveredToAnalysed(FTileInfo Tile);

	TArray<FIntVector> NeighbourOffsets =
	{
		FIntVector(1,0,0),
		FIntVector(-1,0,0),
		FIntVector(0,1,0),
		FIntVector(0,-1,0)
	};

};
