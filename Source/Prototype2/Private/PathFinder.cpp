// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"
#include "GridManager.h"


void PathFinder::Initialise(TMap<FIntVector, AGridCell*>& InGridCells)
{
	GridCells = InGridCells;
}


TArray<FIntVector> PathFinder::FindPath(FIntVector Start, FIntVector End)
{
	StartCoord = Start;
	EndCoord = End;
	TotalMovement = 1000;
	DiscoveredTiles.Empty();
	AnalysedTiles.Empty();
	TileMap.Empty();
	
	TArray<FIntVector> result;

	DiscoverTileForMovement(StartCoord, StartCoord);

	int loopCount = 0;
	while (DiscoveredTiles.Num() > 0)
	{
		loopCount++;
		bool FoundEndPoint = AnalyseNextTile();

		if (loopCount >= 5000) return result;
		
		if (!FoundEndPoint) continue;

		result.Add(TileMap[EndCoord].Coord);
		FIntVector PrevCoord = EndCoord;

		int loopCount2 = 0;
		while (PrevCoord != StartCoord)
		{
			loopCount2++;
			PrevCoord = TileMap[PrevCoord].PreviousTile;
			result.Add(PrevCoord);

			if (loopCount2 >= 5000) return result;
		}
			
		return result;
	}
	
	return result;
}

void PathFinder::DiscoverTileForMovement(FIntVector TileCoord, FIntVector PreviousTile)
{
	FTileInfo TileInfo = FTileInfo();
	TileInfo.Coord = TileCoord;
	TileInfo.EntryCost = GridCells[TileCoord]->MovementCost;
	TileInfo.MinCostToTarget = CalulateMinCostBetweenTiles(TileCoord, EndCoord);
	TileInfo.PreviousTile = PreviousTile;

	int CostFromStart = (!TileMap.IsEmpty())? TileMap[PreviousTile].CostFromStart + TileInfo.EntryCost : 0;
	TileInfo.CostFromStart = CostFromStart;

	if (CostFromStart > TotalMovement) return;
	if (TileCoord != StartCoord && GridCells[TileCoord]->IsOccupied) return;

	TileMap.Add(TileCoord, TileInfo);
	DiscoveredTiles.Add(TileInfo);
}


int PathFinder::CalulateMinCostBetweenTiles(FIntVector Start, FIntVector End)
{
	return abs(Start.X - End.X) + abs(Start.Y - End.Y);
}

FTileInfo PathFinder::PullCheapestTileFromDiscoveredArray()
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

bool PathFinder::AnalyseNextTile()
{
	FTileInfo Tile = PullCheapestTileFromDiscoveredArray();
	MoveTileFromDiscoveredToAnalysed(Tile);

	for (FIntVector Offset : NeighbourOffsets)
	{
		FIntVector NeighbourTile = Tile.Coord + Offset;
		if (!GridCells.Contains(NeighbourTile)) continue;
		if (IsTileAlreadyDiscovered(NeighbourTile)) continue;
		
		DiscoverTileForMovement(NeighbourTile, Tile.Coord);

		if (NeighbourTile == EndCoord) {return true;}
	}
	
	return false;
}

bool PathFinder::IsTileAlreadyDiscovered(FIntVector TileCoord)
{
	return TileMap.Contains(TileCoord);
}

void PathFinder::MoveTileFromDiscoveredToAnalysed(FTileInfo Tile)
{
	DiscoveredTiles.Remove(Tile);
	AnalysedTiles.Add(Tile);
}


TArray<FIntVector> PathFinder::FindMoveableTiles(FIntVector Start, int AvailableMovement)
{
	TArray<FIntVector> WalkableTiles;

	TotalMovement = AvailableMovement;
	StartCoord = Start;
	DiscoveredTiles.Empty();
	AnalysedTiles.Empty();
	TileMap.Empty();

	DiscoverTileForMovement(StartCoord, StartCoord);

	while (DiscoveredTiles.Num() > 0)
	{
		AnalyseTileForMovementAvailability();
	}

	for (FTileInfo Tile : AnalysedTiles)
	{
		WalkableTiles.Add(Tile.Coord);
	}
	
	return WalkableTiles;
}

FTileInfo PathFinder::GetNextTileFromDiscoverableArray()
{
	FTileInfo NextTile = DiscoveredTiles[0];
	MoveTileFromDiscoveredToAnalysed(NextTile);
	return NextTile;
}

void PathFinder::AnalyseTileForMovementAvailability()
{
	FTileInfo Tile = GetNextTileFromDiscoverableArray();

	for (FIntVector Offset : NeighbourOffsets)
	{
		FIntVector NeighbourTile = Tile.Coord + Offset;
		if (!GridCells.Contains(NeighbourTile)) continue;
		if (IsTileAlreadyDiscovered(NeighbourTile)) continue;
		
		DiscoverTileForMovement(NeighbourTile, Tile.Coord);
	}
}

void PathFinder::DiscoverTileForAttack(FIntVector TileCoord, FIntVector PreviousTile)
{
	FTileInfo TileInfo = FTileInfo();
	TileInfo.Coord = TileCoord;
	TileInfo.EntryCost = 1; // Can potentially change this to work for blocking paths of attack with obstacles
	TileInfo.MinCostToTarget = CalulateMinCostBetweenTiles(TileCoord, EndCoord);
	TileInfo.PreviousTile = PreviousTile;

	int CostFromStart = (!TileMap.IsEmpty())? TileMap[PreviousTile].CostFromStart + TileInfo.EntryCost : 0;
	TileInfo.CostFromStart = CostFromStart;

	if (CostFromStart > AttackRange) return;

	TileMap.Add(TileCoord, TileInfo);
	DiscoveredTiles.Add(TileInfo);
}

void PathFinder::AnalyseTileForAttackAvailability()
{
	FTileInfo Tile = GetNextTileFromDiscoverableArray();

	for (FIntVector Offset : NeighbourOffsets)
	{
		FIntVector NeighbourTile = Tile.Coord + Offset;
		if (!GridCells.Contains(NeighbourTile)) continue;
		if (IsTileAlreadyDiscovered(NeighbourTile)) continue;
		
		DiscoverTileForAttack(NeighbourTile, Tile.Coord);
	}
}

TArray<FIntVector> PathFinder::FindAttackableTiles(FIntVector Start, int Range)
{
	TArray<FIntVector> WalkableTiles;
	AttackRange = Range;
	StartCoord = Start;
	DiscoveredTiles.Empty();
	AnalysedTiles.Empty();
	TileMap.Empty();

	DiscoverTileForAttack(StartCoord, StartCoord);

	while (DiscoveredTiles.Num() > 0)
	{
		AnalyseTileForAttackAvailability();
	}

	for (FTileInfo Tile : AnalysedTiles)
	{
		WalkableTiles.Add(Tile.Coord);
	}
	
	return WalkableTiles;
}

TArray<FIntVector> PathFinder::FindPathForEnemy(FIntVector Start, FIntVector End)
{
	StartCoord = Start;
	EndCoord = End;
	TotalMovement = 1000;
	DiscoveredTiles.Empty();
	AnalysedTiles.Empty();
	TileMap.Empty();
	
	TArray<FIntVector> result;

	DiscoverTileForMovement(StartCoord, StartCoord);

	int loopCount = 0;
	while (DiscoveredTiles.Num() > 0)
	{
		loopCount++;
		bool FoundEndPoint = AnalyseTileForEnemyMovement();

		if (loopCount >= 5000) {UE_LOG(LogTemp, Error, TEXT("Infinite loop in analysing tiles")) return result;}
		
		if (!FoundEndPoint) continue;

		result.Add(TileMap[EndCoord].Coord);
		FIntVector PrevCoord = EndCoord;

		int loopCount2 = 0;
		while (PrevCoord != StartCoord)
		{
			loopCount2++;
			PrevCoord = TileMap[PrevCoord].PreviousTile;
			result.Add(PrevCoord);

			if (loopCount2 >= 5000) {UE_LOG(LogTemp, Error, TEXT("Infinite loop back tracing")) return result;}
		}
			
		return result;
	}
	
	return result;
}

bool PathFinder::AnalyseTileForEnemyMovement()
{
	FTileInfo Tile = PullCheapestTileFromDiscoveredArray();
	MoveTileFromDiscoveredToAnalysed(Tile);

	for (FIntVector Offset : NeighbourOffsets)
	{
		FIntVector NeighbourTile = Tile.Coord + Offset;
		if (!GridCells.Contains(NeighbourTile)) continue;
		if (IsTileAlreadyDiscovered(NeighbourTile)) continue;
		
		DiscoverTileForEnemyMovement(NeighbourTile, Tile.Coord);

		if (NeighbourTile == EndCoord) {return true;}
	}
	
	return false;
}

// discovery for enemy movement is only different in that it ignores that the end target tile is occupied
// EnemyEntity class has logic for not moving onto the final occupied tile
void PathFinder::DiscoverTileForEnemyMovement(FIntVector TileCoord, FIntVector PreviousTile)
{
	FTileInfo TileInfo = FTileInfo();
	TileInfo.Coord = TileCoord;
	TileInfo.EntryCost = GridCells[TileCoord]->MovementCost;
	TileInfo.MinCostToTarget = CalulateMinCostBetweenTiles(TileCoord, EndCoord);
	TileInfo.PreviousTile = PreviousTile;

	int CostFromStart = (!TileMap.IsEmpty())? TileMap[PreviousTile].CostFromStart + TileInfo.EntryCost : 0;
	TileInfo.CostFromStart = CostFromStart;

	if (CostFromStart > TotalMovement) return;
	if (TileCoord != StartCoord && TileCoord != EndCoord && GridCells[TileCoord]->IsOccupied) return;

	TileMap.Add(TileCoord, TileInfo);
	DiscoveredTiles.Add(TileInfo);
}

// returns the path to the point furthest from the target coord that is within the attack Range of that target coord
// the reasoning behind this is that any tile that the target coord can attack is also one it can be attacked from
TArray<FIntVector> PathFinder::FindPathToPointInRangeOFTarget(FIntVector Start, FIntVector End, int Range)
{
	// find potential tiles to move to
	TArray<FIntVector> TilesInAttackRange = FindAttackableTiles(End, Range);
	// limit the search to the furthest tiles from the target (because why move closer than is necessary)
	TArray<FIntVector> PerimeterTiles = GetTilesOnPerimeter(TilesInAttackRange);

	// get the tile in the perimeter that's closest to the start coord
	// if none of the tiles are valid then just return the path that takes the start to the end
	FIntVector ClosestTile = FIntVector(0);
	if (!GetTileInArrayClosestToTarget(PerimeterTiles, Start, ClosestTile)) return FindPathForEnemy(Start, End);

	// return the path to the chosen tile
	return FindPathForEnemy(Start, ClosestTile);
}

TArray<FIntVector> PathFinder::GetTilesOnPerimeter(TArray<FIntVector>& Tiles)
{
	TArray<FIntVector> result;

	for (int i = 0; i < Tiles.Num(); i++)
	{
		int NumOfNeighbours = 0;
		if (Tiles.Contains(Tiles[i] + NeighbourOffsets[0])) NumOfNeighbours++;
		if (Tiles.Contains(Tiles[i] + NeighbourOffsets[1])) NumOfNeighbours++;
		if (Tiles.Contains(Tiles[i] + NeighbourOffsets[2])) NumOfNeighbours++;
		if (Tiles.Contains(Tiles[i] + NeighbourOffsets[3])) NumOfNeighbours++;
		if (NumOfNeighbours < 4) result.Add(Tiles[i]);
	}
	
	return result;
}

// returns true if a valid tile was found (protection against all tiles being occupied)
bool PathFinder::GetTileInArrayClosestToTarget(TArray<FIntVector>& Tiles, FIntVector Target, FIntVector& OutCoord)
{
	bool result = false;
	int ShortestDist = 10000;

	for (int i = 0; i < Tiles.Num(); i++)
	{
		int Dist = abs(Tiles[i].X - Target.X) + abs(Tiles[i].Y - Target.Y);
		if (Dist >= ShortestDist) continue;
		if (GridCells[Tiles[i]]->IsOccupied) continue;
		ShortestDist = Dist;
		OutCoord = Tiles[i];
		result = true;
	}
	
	return result;
}
