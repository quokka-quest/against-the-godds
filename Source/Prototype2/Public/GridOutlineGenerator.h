// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridOutlineActor.h"
#include "GridData.h"
#include "GridCellBase.h"

struct FOutlineCellInfo
{
	FIntVector2 CellCoord;
	bool HasPosXNeighbour = false;
	bool HasPosYNeighbour = false;
	bool HasNegXNeighbour = false;
	bool HasNegYNeighbour = false;
};

enum EdgeDirection
{
	PosX,
	NegX,
	PosY,
	NegY
};

/**
 * 
 */
class PROTOTYPE2_API GridOutlineGenerator
{
public:
	GridOutlineGenerator(AGridOutlineActor* InOutlineActor):GridOutlineActor(InOutlineActor){};

	void GenerateFullGridOutline(TMap<FIntVector2, AGridCellBase*>& GridCells, float Height, float GridCellSizeX, float GridCellSizeY);

	void GenerateOutlineFromGridData(FGridData& GridData, float GridCellSizeX, float GridCellSizeY);

	void GenerateOutlineFromCoordArray(TArray<FIntVector2>& CellArray, FIntVector2 OriginCoord, float GridCellSizeX, float GridCellSizeY);
	
protected:
	AGridOutlineActor* GridOutlineActor;

	float LineWidth = 10.0f;
	float HalfCellSizeX = 1.0f;
	float HalfCellSizeY = 1.0f;

	TMap<FVector, FOutlineEdge> StartEdgeMapHorizontal; // used for forward searching through the outline
	TMap<FVector, FOutlineEdge> StartEdgeMapVertical;
	TMap<FVector, FOutlineEdge> EndEdgeMapHorizontal; // used for backward searching through the outline
	TMap<FVector, FOutlineEdge> EndEdgeMapVertical;
	TMap<FVector, FOutlineVertexInfo> CompressedStartEndMap; // the final compressed Start-End map to be passed for mesh generation

	// helper functions
	TArray<FOutlineCellInfo> GetOutlineInfo(TArray<FIntVector2>& Cells);
	void FillBasicStartEndMaps(TArray<FOutlineCellInfo>& CellOutlineInfo);
	void CompressStartEndMap();
	void AddEdgeToStartAndEndMap(FVector Pos, EdgeDirection Dir);
	FVector ComputerMiterOffset(const FVector& SharedVertex, const FOutlineEdge& PrimaryEdge, const FOutlineEdge& SecondaryEdge, bool StartMiter);
};
