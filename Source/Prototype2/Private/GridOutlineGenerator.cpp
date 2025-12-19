// Fill out your copyright notice in the Description page of Project Settings.


#include "GridOutlineGenerator.h"

// this function is for generating the outline of the entire grid used in the level
void GridOutlineGenerator::GenerateFullGridOutline(TMap<FIntVector2, AGridCellBase*>& GridCells, float Height, float GridCellSizeX, float GridCellSizeY)
{
	// find all the perimeter cells by checking if neighbours exist (a cell with all 4 neighbours is not on the perimeter)
	TArray<FIntVector2> Cells;
	GridCells.GenerateKeyArray(Cells);
	TArray<FOutlineCellInfo> PerimeterCells = GetOutlineInfo(Cells);

	// construct the map for start and end points of the grid highlight around each cell individually
	// NOTE: 'start' to 'end' must have a consistent rotation around the cell (clockwise or anti-clockwise)
	// The rotation means each 'End' will also be a 'Start' for the adjacent cell's outline which is important for the logic in the while loop below
	TMap<FVector, FVector> FullStartEndPoints;
	float HalfCellSizeX = GridCellSizeX * 0.5f;
	float HalfCellSizeY = GridCellSizeY * 0.5f;
	for (FOutlineCellInfo Cell : PerimeterCells)
	{
		if (!Cell.HasPosXNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, HalfCellSizeY, Height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, -HalfCellSizeY, Height);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasNegXNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, -HalfCellSizeY, Height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, HalfCellSizeY, Height);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasPosYNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, HalfCellSizeY, Height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, HalfCellSizeY, Height);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasNegYNeighbour)
		{
			FVector Start = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(HalfCellSizeX, -HalfCellSizeY, Height);
			FVector End = GridCells[Cell.CellCoord]->GetActorLocation() + FVector(-HalfCellSizeX, -HalfCellSizeY, Height);
			FullStartEndPoints.Add(Start, End);
		}
	}
	
	// construct a compressed map of the grid outline (adjacent cells with an outline in the same direction will be compressed to one outline)
	TMap<FVector, FVector> CompressedStartEndMap = CompressStartEndMap(FullStartEndPoints);

	GridOutlineActor->BuildOutlineMesh(CompressedStartEndMap, LineWidth);
}

// this function generates an outline when given GridData (for outlining attack AOEs and the cell highlight)
void GridOutlineGenerator::GenerateOutlineFromGridData(FGridData& GridData, float CellSizeX, float CellSizeY)
{
	TArray<FIntVector2> Offsets = GridData.GetSelectedCellOffsets();
	TArray<FOutlineCellInfo> PerimeterOffsets = GetOutlineInfo(Offsets);

	float HalfCellSizeX = CellSizeX * 0.5f;
	float HalfCellSizeY = CellSizeY * 0.5f;
	TMap<FVector, FVector> FullStartEndPoints;
	for (FOutlineCellInfo Cell : PerimeterOffsets)
	{
		FVector Pos = FVector(Cell.CellCoord.X * CellSizeX, Cell.CellCoord.Y * CellSizeY, 0.0f);
		
		if (!Cell.HasPosXNeighbour)
		{
			FVector Start = Pos + FVector(HalfCellSizeX, HalfCellSizeY, 0.0f);
			FVector End = Pos + FVector(HalfCellSizeX, -HalfCellSizeY, 0.0f);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasNegXNeighbour)
		{
			FVector Start = Pos + FVector(-HalfCellSizeX, -HalfCellSizeY, 0.0f);
			FVector End = Pos + FVector(-HalfCellSizeX, HalfCellSizeY, 0.0f);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasPosYNeighbour)
		{
			FVector Start = Pos + FVector(-HalfCellSizeX, HalfCellSizeY, 0.0f);
			FVector End = Pos + FVector(HalfCellSizeX, HalfCellSizeY, 0.0f);
			FullStartEndPoints.Add(Start, End);
		}
		if (!Cell.HasNegYNeighbour)
		{
			FVector Start = Pos + FVector(HalfCellSizeX, -HalfCellSizeY, 0.0f);
			FVector End = Pos + FVector(-HalfCellSizeX, -HalfCellSizeY, 0.0f);
			FullStartEndPoints.Add(Start, End);
		}
	}

	TMap<FVector, FVector> CompressedStartEndMap = CompressStartEndMap(FullStartEndPoints);

	GridOutlineActor->BuildOutlineMesh(CompressedStartEndMap, LineWidth);
}

// This function takes in an array of cells and outputs the outline information (cells on the perimeter and which sides of the cell need outlining)
TArray<FOutlineCellInfo> GridOutlineGenerator::GetOutlineInfo(TArray<FIntVector2>& Cells)
{
	TArray<FOutlineCellInfo> PerimeterOffsets;

	for (FIntVector2 Cell : Cells)
	{
		FOutlineCellInfo CellInfo;
		CellInfo.CellCoord = Cell;
		int NeighbourCount = 0;
		if (Cells.Contains(Cell + FIntVector2(1,0))) { NeighbourCount++; CellInfo.HasPosXNeighbour = true; }
		if (Cells.Contains(Cell + FIntVector2(-1,0))) { NeighbourCount++; CellInfo.HasNegXNeighbour = true; }
		if (Cells.Contains(Cell + FIntVector2(0,1))) { NeighbourCount++; CellInfo.HasPosYNeighbour = true; }
		if (Cells.Contains(Cell + FIntVector2(0,-1))) { NeighbourCount++; CellInfo.HasNegYNeighbour = true; }
		if (NeighbourCount < 4) PerimeterOffsets.Add(CellInfo);
	}

	return PerimeterOffsets;
}

// this function takes in a map (key are the Start Points and Value are the End Points)
// It returns a compressed version of that map (all highlight segments that are next to each other are combined into one longer segment)
TMap<FVector, FVector> GridOutlineGenerator::CompressStartEndMap(TMap<FVector, FVector>& FullStartEndMap)
{
	TMap<FVector, FVector> CompressedStartEndMap;
	TArray<FVector> StartPointArray;
	FullStartEndMap.GenerateKeyArray(StartPointArray);

	// Primary loop for going through every start point in the full map
	while (StartPointArray.Num() > 0)
	{
		FVector Start = StartPointArray[0];
		FVector End = FullStartEndMap[Start];
		FVector Dir = (End-Start).GetSafeNormal();
		StartPointArray.Remove(Start);

		FVector NextStart = End;
		FVector NextEnd = FullStartEndMap[NextStart];
		FVector NextDir = (NextEnd-NextStart).GetSafeNormal();

		// loop for compressing adjacent segments if they have the same direction
		while (NextDir == Dir)
		{
			StartPointArray.Remove(NextStart);
			End = NextEnd;

			NextStart = End;
			NextEnd = FullStartEndMap[NextStart];
			NextDir = (NextEnd-NextStart).GetSafeNormal();
		}
		CompressedStartEndMap.Add(Start, End);
	}

	return CompressedStartEndMap;
}
