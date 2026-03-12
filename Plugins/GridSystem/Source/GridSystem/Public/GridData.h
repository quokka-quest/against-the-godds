// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridData.Generated.h"

UENUM(Blueprintable, BlueprintType)
enum EPatternRotation
{
	R0,
	R90,
	R180,
	R270
};

USTRUCT(BlueprintType)
struct FGridData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridData")
	int32 Columns = 4;
	UPROPERTY(Transient)
	int32 CachedColumns = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridData")
	int32 Rows = 4;
	UPROPERTY(Transient)
	int32 CachedRows = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridData")
	FIntVector2 OriginCellGridCoord = FIntVector2(0, 0);
	UPROPERTY(Transient)
	FIntVector2 CachedOriginCellGridCoord = FIntVector2(0, 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridData")
	float CellSize = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridData")
	TArray<bool> AllCellsArray;

	void ResizeGrid()
	{
		if (Columns <= 0 || Rows <= 0)
		{
			AllCellsArray.Empty();
			return;
		}

		AllCellsArray.SetNum(Columns * Rows);
	}

	bool IsCellSelected(int32 X, int32 Y) const
	{
		int32 Index = Y * Columns + X;
		return AllCellsArray.IsValidIndex(Index) ? AllCellsArray[Index] : false;
	}

	void ToggleCell(int32 X, int32 Y)
	{
		int32 Index = Y * Columns + X;
		if (AllCellsArray.IsValidIndex(Index))
		{
			AllCellsArray[Index] = !AllCellsArray[Index];
		}
	}

	FIntVector2 GetCellsGridCoord(int32 CellIndex) const
	{
		int32 XCoord = CellIndex % Columns;
		int32 YCoord = (CellIndex - XCoord) / Columns;
		return FIntVector2(XCoord, YCoord);
	}

	FIntVector2 GetCellsOffsetFromOriginCell(int32 CellIndex) const
	{
		FIntVector2 CellsGridCoord = GetCellsGridCoord(CellIndex);
		return OriginCellGridCoord - CellsGridCoord;
	}

	bool IsCellTheOrigin(int32 CellIndex) const
	{
		return OriginCellGridCoord == GetCellsGridCoord(CellIndex);
	}

	TArray<FIntVector2> GetSelectedCellOffsets() const
	{
		TArray<FIntVector2> SelectedCellOffsets;
		SelectedCellOffsets.Add(FIntVector2(0,0));
		
		for (int32 i = 0; i < AllCellsArray.Num(); i++)
		{
			if (!AllCellsArray[i] || IsCellTheOrigin(i)) continue;

			SelectedCellOffsets.Add(GetCellsOffsetFromOriginCell(i));
		}

		return SelectedCellOffsets;
	}
	
};

USTRUCT(BlueprintType)
struct FPathingData
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Actor;
	UPROPERTY()
	TMap<TEnumAsByte<EPatternRotation>, FGridData> ActorRotations;
	UPROPERTY()
	FGridData RotationSweep;
	UPROPERTY()
	TEnumAsByte<EPatternRotation> CurrentRotation;
};

USTRUCT(BlueprintType)
struct FPathInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FIntVector2 StartingCoord;
	UPROPERTY()
	FIntVector2 CoordToMoveTo;
	UPROPERTY()
	TEnumAsByte<EPatternRotation> StartingRot;
	UPROPERTY()
	TEnumAsByte<EPatternRotation> RotToChangeTo;
	UPROPERTY()
	int HazardPenaltyFromStart;
};

UENUM(BlueprintType)
enum EPathingRules
{
	ExcludeOccupiedCells,
	ExcludeHazardCells,
	TryPathAroundHazards,
	RangeIsAvailableMovement,
	MustFitOnTarget,
	StraightLine,
};

UENUM(BlueprintType)
enum EAttackRules
{
	ObeyTraversalRules,
	IgnoreLineOfSight,
	StraightLineOnly,
	MustFitOnTargetCell,
	PatternIsPath,
};