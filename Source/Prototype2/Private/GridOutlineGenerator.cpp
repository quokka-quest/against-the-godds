// Fill out your copyright notice in the Description page of Project Settings.


#include "GridOutlineGenerator.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////// Main Generation functions

// this function is for generating the outline of the entire grid used in the level
void GridOutlineGenerator::GenerateFullGridOutline(TMap<FIntVector2, AGridCellBase*>& GridCells, float Height, float GridCellSizeX, float GridCellSizeY)
{
	StartEdgeMapHorizontal.Empty();
	StartEdgeMapVertical.Empty();
	EndEdgeMapHorizontal.Empty();
	EndEdgeMapVertical.Empty();
	CompressedStartEndMap.Empty();
	HalfCellSizeX = GridCellSizeX * 0.5f;
	HalfCellSizeY = GridCellSizeY * 0.5f;
	
	// find all the perimeter cells by checking if neighbours exist (a cell with all 4 neighbours is not on the perimeter)
	TArray<FIntVector2> Cells;
	GridCells.GenerateKeyArray(Cells);
	TArray<FOutlineCellInfo> PerimeterCells = GetOutlineInfo(Cells);

	// build the line segment maps
	FillBasicStartEndMaps(PerimeterCells);
	CompressStartEndMap();

	TArray<FOutlineEdge> EdgeArray;
	for (auto& Edge : CompressedStartEndMap)
	{
		for (FOutlineEdge Edged : Edge.Value.Edges)
		{
			EdgeArray.Add(Edged);
		}
	}
	GridOutlineActor->BuildOutlineMesh(EdgeArray, LineWidth);
}

// this function generates an outline when given GridData (for outlining attack AOEs and the cell highlight)
void GridOutlineGenerator::GenerateOutlineFromGridData(FGridData& GridData, float GridCellSizeX, float GridCellSizeY)
{
	StartEdgeMapHorizontal.Empty();
	StartEdgeMapVertical.Empty();
	EndEdgeMapHorizontal.Empty();
	EndEdgeMapVertical.Empty();
	CompressedStartEndMap.Empty();
	HalfCellSizeX = GridCellSizeX * 0.5f;
	HalfCellSizeY = GridCellSizeY * 0.5f;
	
	// get the perimeter information
	TArray<FIntVector2> Offsets = GridData.GetSelectedCellOffsets();
	TArray<FOutlineCellInfo> PerimeterInfo = GetOutlineInfo(Offsets);

	// construct and compress a Start-End map
	FillBasicStartEndMaps(PerimeterInfo);
	CompressStartEndMap();

	TArray<FOutlineEdge> EdgeArray;
	for (auto& Edge : CompressedStartEndMap)
	{
		for (FOutlineEdge Edged : Edge.Value.Edges)
		{
			EdgeArray.Add(Edged);
		}
	}
	GridOutlineActor->BuildOutlineMesh(EdgeArray, LineWidth);
}

// this function generates an outline for an array of cell coordinates
void GridOutlineGenerator::GenerateOutlineFromCoordArray(TArray<FIntVector2>& CellArray, FIntVector2 OriginCoord, float GridCellSizeX, float GridCellSizeY)
{
	StartEdgeMapHorizontal.Empty();
	StartEdgeMapVertical.Empty();
	EndEdgeMapHorizontal.Empty();
	EndEdgeMapVertical.Empty();
	CompressedStartEndMap.Empty();
	HalfCellSizeX = GridCellSizeX * 0.5f;
	HalfCellSizeY = GridCellSizeY * 0.5f;
	
	TArray<FIntVector2> CentredCells;

	for (FIntVector2 Cell : CellArray)
	{
		CentredCells.Add(Cell - OriginCoord);
	}
	
	// get the perimeter information
	TArray<FOutlineCellInfo> PerimeterInfo = GetOutlineInfo(CentredCells);

	// construct and compress a Start-End map
	FillBasicStartEndMaps(PerimeterInfo);
	CompressStartEndMap();

	TArray<FOutlineEdge> EdgeArray;
	for (auto& Edge : CompressedStartEndMap)
	{
		for (FOutlineEdge Edged : Edge.Value.Edges)
		{
			EdgeArray.Add(Edged);
		}
	}
	GridOutlineActor->BuildOutlineMesh(EdgeArray, LineWidth);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////// Helper Functions

// This function takes in an array of cells and outputs the outline information
// The output array contains the cells on the perimeter and which sides of those cell need outlining
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

// construct the map for line segments of the grid highlight around each cell individually
void GridOutlineGenerator::FillBasicStartEndMaps(TArray<FOutlineCellInfo>& CellOutlineInfo)
{
	for (FOutlineCellInfo Cell : CellOutlineInfo)
	{
		FVector Pos = FVector(Cell.CellCoord.X * HalfCellSizeX * 2.0f, Cell.CellCoord.Y * HalfCellSizeY * 2.0f, 0.0f);
		
		if (!Cell.HasPosXNeighbour) AddEdgeToStartAndEndMap(Pos, PosX);
		if (!Cell.HasNegXNeighbour) AddEdgeToStartAndEndMap(Pos, NegX);
		if (!Cell.HasPosYNeighbour) AddEdgeToStartAndEndMap(Pos, PosY);
		if (!Cell.HasNegYNeighbour) AddEdgeToStartAndEndMap(Pos, NegY);
	}
}


// this function takes in a map (key are the Start Points and Value are the End Points)
// It returns a compressed version of that map (all highlight segments that are next to each other are combined into one longer segment)
void GridOutlineGenerator::CompressStartEndMap()
{
	// the Array to loop through (makes sure every point has been copied to the compressed map)
	TArray<FVector> HorizontalStartPoints;
	StartEdgeMapHorizontal.GenerateKeyArray(HorizontalStartPoints);

	// Primary loop for going through every start point in the full map
	while (HorizontalStartPoints.Num() > 0)
	{
		FVector Start = HorizontalStartPoints[0];
		FOutlineEdge Edge = StartEdgeMapHorizontal[Start];
		HorizontalStartPoints.Remove(Start);

		FVector NextStart = Edge.End;
		FOutlineEdge NextEdge;
		bool NextMatchesDir = StartEdgeMapHorizontal.Contains(NextStart);

		// loop for compressing adjacent segments if they have the same direction (forward search)
		while (NextMatchesDir)
		{
			NextEdge = StartEdgeMapHorizontal[NextStart];
			HorizontalStartPoints.Remove(NextStart);
			Edge.End = NextEdge.End;

			NextStart = NextEdge.End;
			NextMatchesDir = StartEdgeMapHorizontal.Contains(NextStart);
		}

		// calculate the 'EndMiter' (offset for mesh generation)
		if (StartEdgeMapVertical.Contains(Edge.End) && EndEdgeMapVertical.Contains(Edge.End)) {Edge.EndMiter = FVector::ZeroVector;}
		else if (StartEdgeMapVertical.Contains(Edge.End)) {Edge.EndMiter = ComputerMiterOffset(Edge.End, Edge, StartEdgeMapVertical[Edge.End], false);}
		else if (EndEdgeMapVertical.Contains(Edge.End)) {Edge.EndMiter = ComputerMiterOffset(Edge.End, Edge, EndEdgeMapVertical[Edge.End], false);}
		else {UE_LOG(LogTemp, Error, TEXT("GridOutlineGenerator.cpp->CompressStartEndMap: Something went wrong with 'End miter' calc"))}

		// loop for compressing adjacent segments if they have the same direction (backward search)
		FVector NextEnd = Start;
		NextMatchesDir = EndEdgeMapHorizontal.Contains(NextEnd);
		while (NextMatchesDir)
		{
			NextEdge = EndEdgeMapHorizontal[NextEnd];
			if (HorizontalStartPoints.Contains(NextEnd)) HorizontalStartPoints.Remove(NextEnd);
			Edge.Start = NextEdge.Start;

			NextEnd = NextEdge.Start;
			NextMatchesDir = EndEdgeMapHorizontal.Contains(NextEnd);
		}

		// calculate the 'StartMiter' (offset for mesh generation)
		if (StartEdgeMapVertical.Contains(Edge.Start) && EndEdgeMapVertical.Contains(Edge.Start)) Edge.StartMiter = FVector::ZeroVector;
		else if (StartEdgeMapVertical.Contains(Edge.Start)) Edge.StartMiter = ComputerMiterOffset(Edge.Start, Edge, StartEdgeMapVertical[Edge.Start], true);
		else if (EndEdgeMapVertical.Contains(Edge.Start)) Edge.StartMiter = ComputerMiterOffset(Edge.Start, Edge, EndEdgeMapVertical[Edge.Start], true);
		else {UE_LOG(LogTemp, Error, TEXT("GridOutlineGenerator.cpp->CompressStartEndMap: Something went wrong with 'Start miter' calc"))}
		
		CompressedStartEndMap.FindOrAdd(Edge.Start).Edges.Add(Edge);
		//UE_LOG(LogTemp, Warning, TEXT("Edge Info-> Start: %f, %f"), Edge.Start.X, Edge.Start.Y)
	}

	/////////////////////////////////////////////////////////////////////////////////////////////// Loop for the vertical edges
	// the Array to loop through (makes sure every point has been copied to the compressed map)
	TArray<FVector> VerticalStartPoints;
	StartEdgeMapVertical.GenerateKeyArray(VerticalStartPoints);

	// Primary loop for going through every start point in the full map
	while (VerticalStartPoints.Num() > 0)
	{
		FVector Start = VerticalStartPoints[0];
		FOutlineEdge Edge = StartEdgeMapVertical[Start];
		VerticalStartPoints.Remove(Start);

		FVector NextStart = Edge.End;
		FOutlineEdge NextEdge;
		bool NextMatchesDir = StartEdgeMapVertical.Contains(NextStart);

		// loop for compressing adjacent segments if they have the same direction (forward search)
		while (NextMatchesDir)
		{
			NextEdge = StartEdgeMapVertical[NextStart];
			VerticalStartPoints.Remove(NextStart);
			Edge.End = NextEdge.End;

			NextStart = NextEdge.End;
			NextMatchesDir = StartEdgeMapVertical.Contains(NextStart);
		}

		// calculate the 'EndMiter' (offset for mesh generation)
		if (StartEdgeMapHorizontal.Contains(Edge.End) && EndEdgeMapHorizontal.Contains(Edge.End)) {Edge.EndMiter = FVector::ZeroVector;}
		else if (StartEdgeMapHorizontal.Contains(Edge.End)) {Edge.EndMiter = ComputerMiterOffset(Edge.End, Edge, StartEdgeMapHorizontal[Edge.End], false);}
		else if (EndEdgeMapHorizontal.Contains(Edge.End)) {Edge.EndMiter = ComputerMiterOffset(Edge.End, Edge, EndEdgeMapHorizontal[Edge.End], false);}
		else {UE_LOG(LogTemp, Error, TEXT("GridOutlineGenerator.cpp->CompressStartEndMap: Something went wrong with 'End miter' calc"))}

		// loop for compressing adjacent segments if they have the same direction (backward search)
		FVector NextEnd = Start;
		NextMatchesDir = EndEdgeMapVertical.Contains(NextEnd);
		while (NextMatchesDir)
		{
			NextEdge = EndEdgeMapVertical[NextEnd];
			if (VerticalStartPoints.Contains(NextEnd)) VerticalStartPoints.Remove(NextEnd);
			Edge.Start = NextEdge.Start;

			NextEnd = NextEdge.Start;
			NextMatchesDir = EndEdgeMapVertical.Contains(NextEnd);
		}

		// calculate the 'StartMiter' (offset for mesh generation)
		if (StartEdgeMapHorizontal.Contains(Edge.Start) && EndEdgeMapHorizontal.Contains(Edge.Start)) Edge.StartMiter = FVector::ZeroVector;
		else if (StartEdgeMapHorizontal.Contains(Edge.Start)) Edge.StartMiter = ComputerMiterOffset(Edge.Start, Edge, StartEdgeMapHorizontal[Edge.Start], true);
		else if (EndEdgeMapHorizontal.Contains(Edge.Start)) Edge.StartMiter = ComputerMiterOffset(Edge.Start, Edge, EndEdgeMapHorizontal[Edge.Start], true);
		else {UE_LOG(LogTemp, Error, TEXT("GridOutlineGenerator.cpp->CompressStartEndMap: Something went wrong with 'Start miter' calc"))}
		
		CompressedStartEndMap.FindOrAdd(Edge.Start).Edges.Add(Edge);
	}
}

void GridOutlineGenerator::AddEdgeToStartAndEndMap(FVector Pos, EdgeDirection Dir)
{
	// Start->End moves in the positive X direction when horizontal and positive Y direction when vertical
	float StartXMult = (Dir == PosX)? 1.0f : -1.0f;
	float StartYMult = (Dir == PosY)? 1.0f : -1.0f;
	float EndXMult = (Dir == NegX)? -1.0f : 1.0f;
	float EndYMult = (Dir == NegY)? -1.0f : 1.0f;
	
	FVector Start = Pos + FVector(HalfCellSizeX * StartXMult, HalfCellSizeY * StartYMult, 0.0f);
	FVector End = Pos + FVector(HalfCellSizeX * EndXMult, HalfCellSizeY * EndYMult, 0.0f);
	FOutlineEdge Edge;
	Edge.Start = Start;
	Edge.End = End;

	if (Dir == PosY || Dir == NegY)
	{
		StartEdgeMapHorizontal.Add(Edge.Start) = Edge;
		EndEdgeMapHorizontal.Add(Edge.End) = Edge;
	}
	else
	{
		StartEdgeMapVertical.Add(Edge.Start) = Edge;
		EndEdgeMapVertical.Add(Edge.End) = Edge;
	}
}

// Miter calculations need a continuous flow direction
FVector GridOutlineGenerator::ComputerMiterOffset(const FVector& SharedVertex, const FOutlineEdge& PrimaryEdge, const FOutlineEdge& SecondaryEdge, bool StartMiter)
{
	FVector PrimeDir = (PrimaryEdge.End - PrimaryEdge.Start).GetSafeNormal();
	FVector SecondDir;

	if (StartMiter)
	{
		SecondDir = (SecondaryEdge.Start == SharedVertex)? (SecondaryEdge.Start-SecondaryEdge.End).GetSafeNormal() : (SecondaryEdge.End-SecondaryEdge.Start).GetSafeNormal();
	}
	else
	{
		SecondDir = (SecondaryEdge.Start == SharedVertex)? (SecondaryEdge.End-SecondaryEdge.Start).GetSafeNormal() : (SecondaryEdge.Start-SecondaryEdge.End).GetSafeNormal();
	}

	FVector PrimePerp = FVector::CrossProduct(PrimeDir, FVector::UpVector).GetSafeNormal();
	FVector SecondPerp = FVector::CrossProduct(SecondDir, FVector::UpVector).GetSafeNormal();

	FVector MiterDir = (PrimePerp + SecondPerp).GetSafeNormal();
	float MiterMag = FVector::DotProduct(MiterDir, SecondPerp);

	return MiterDir * (LineWidth * 0.5f) / MiterMag;
}
