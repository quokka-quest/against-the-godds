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

/**
 * 
 */
class PROTOTYPE2_API GridOutlineGenerator
{
public:
	GridOutlineGenerator(AGridOutlineActor* InOutlineActor):GridOutlineActor(InOutlineActor){};

	void GenerateFullGridOutline(TMap<FIntVector2, AGridCellBase*>& GridCells, float Height, float GridCellSizeX, float GridCellSizeY);

	void GenerateOutlineFromGridData(FGridData& GridData, float CellSizeX, float CellSizeY);
	
protected:
	AGridOutlineActor* GridOutlineActor;

	float LineWidth = 10.0f;

	TArray<FOutlineCellInfo> GetOutlineInfo(TArray<FIntVector2>& Cells);
	TMap<FVector, FVector> CompressStartEndMap(TMap<FVector, FVector>& FullStartEndMap);
};
