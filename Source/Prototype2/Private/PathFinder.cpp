// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"
#include "GridManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APathFinder::APathFinder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

TArray<FIntVector> APathFinder::FindPath(FIntVector Start, FIntVector End)
{
	StartCoord = Start;
	EndCoord = End;
	DiscoveredTiles.Empty();
	AnalysedTiles.Empty();
	TileMap.Empty();
	
	TArray<FIntVector> result;
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	DiscoverTile(StartCoord, StartCoord);

	int loopCount = 0;
	while (DiscoveredTiles.Num() > 0)
	{
		loopCount++;
		bool FoundEndPoint = AnalyseNextTile();

		if (loopCount >= 500) {UE_LOG(LogTemp, Error, TEXT("Infinite loop in analysing tiles")) return result;}
		
		if (!FoundEndPoint) continue;

		result.Add(TileMap[EndCoord].Coord);
		FIntVector PrevCoord = EndCoord;

		int loopCount2 = 0;
		while (PrevCoord != StartCoord)
		{
			loopCount2++;
			PrevCoord = TileMap[PrevCoord].PreviousTile;
			result.Add(PrevCoord);

			if (loopCount2 >= 500) {UE_LOG(LogTemp, Error, TEXT("Infinite loop back tracing")) return result;}
		}
			
		return result;
	}
	
	return result;
}

void APathFinder::DiscoverTile(FIntVector TileCoord, FIntVector PreviousTile)
{
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	FTileInfo TileInfo = FTileInfo();
	TileInfo.Coord = TileCoord;
	TileInfo.EntryCost = 1;
	TileInfo.MinCostToTarget = CalulateMinCostBetweenTiles(TileCoord, EndCoord);
	TileInfo.PreviousTile = PreviousTile;

	int CostFromStart = (!TileMap.IsEmpty())? TileMap[PreviousTile].CostFromStart + TileInfo.EntryCost : 0;
	TileInfo.CostFromStart = CostFromStart;

	TileMap.Add(TileCoord, TileInfo);
	DiscoveredTiles.Add(TileInfo);
}


int APathFinder::CalulateMinCostBetweenTiles(FIntVector Start, FIntVector End)
{
	return abs(Start.X - End.X) + abs(Start.Y - End.Y);
}

FTileInfo APathFinder::PullCheapestTileFromDiscoveredArray()
{
	FTileInfo CheapestTile = FTileInfo();
	int CheapestCost = 10000;

	for (FTileInfo Tiles : DiscoveredTiles)
	{
		if (Tiles.MinCostToTarget >= CheapestCost) continue;
		CheapestCost = Tiles.MinCostToTarget;
		CheapestTile = Tiles;
	}

	return CheapestTile;
}

bool APathFinder::AnalyseNextTile()
{
	FTileInfo Tile = PullCheapestTileFromDiscoveredArray();
	MoveTileFromDiscoveredToAnalysed(Tile);
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	for (FIntVector Offset : NeighbourOffsets)
	{
		FIntVector NeighbourTile = Tile.Coord + Offset;
		if (!GridManager->GridCells.Contains(NeighbourTile)) continue;
		if (IsTileAlreadyDiscovered(NeighbourTile)) continue;
		
		DiscoverTile(NeighbourTile, Tile.Coord);

		if (NeighbourTile == EndCoord) {return true;}
	}
	
	return false;
}

bool APathFinder::IsTileAlreadyDiscovered(FIntVector TileCoord)
{
	return TileMap.Contains(TileCoord);
}

void APathFinder::MoveTileFromDiscoveredToAnalysed(FTileInfo Tile)
{
	DiscoveredTiles.Remove(Tile);
	AnalysedTiles.Add(Tile);
}





