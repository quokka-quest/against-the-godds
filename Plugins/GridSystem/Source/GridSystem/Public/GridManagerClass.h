// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridData.h"
#include "GridCellBase.h"
#include "GridManagerClass.generated.h"

UCLASS()
class GRIDSYSTEM_API AGridManagerClass : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Grid System")
	FGridData GridData;

	UPROPERTY(EditAnywhere, Category = "Grid System")
	TSubclassOf<AGridCellBase> GridCellActor;

	UPROPERTY(EditAnywhere, Category = "Grid System")
	float GridCellSizeX;
	UPROPERTY(EditAnywhere, Category = "Grid System")
	float GridCellSizeY;

	UPROPERTY()
	TMap<FIntVector2, AGridCellBase*> GridCells;

	UFUNCTION(CallInEditor, Category="Grid System")
	void RegenerateGrid();

	UFUNCTION(CallInEditor, Category="Grid System")
	void PrintAllCellCoords();

	// PathFinding functions
	UFUNCTION(BlueprintCallable, Category="Grid System")
	void ResetWalkableAndAttackableOnAllCells();
	
	UFUNCTION(BlueprintCallable, Category="Grid System")
	TArray<FIntVector2> GetCellsInRange(FIntVector2 StartCoord, int AvailableMovement, FPathingData PathingData, TArray<TEnumAsByte<EPathingRules>> Rules);

	UFUNCTION(BlueprintCallable, Category="Grid System")
	TArray<FIntVector2> GetCellsInAttackArea(FIntVector2 Target, FGridData AttackPattern, EPatternRotation Rotation, FPathingData PathingData);

	UFUNCTION(BlueprintCallable, Category="Grid System")
	TArray<FPathInfo> GetPathToPointInRangeOfTarget(FIntVector2 Start, FIntVector2 End, int Range, FPathingData PathingData);

	UFUNCTION(BlueprintCallable, Category="Grid System")
	TArray<FPathInfo> GetPathBetweenCoords(FIntVector2 Start, FIntVector2 End, int AvailableMovement, FPathingData PathingData);
	
	UFUNCTION(BlueprintCallable, Category="Grid System")
	TArray<FPathInfo> PathFindBetweenTwoCoords(FIntVector2 Start, FIntVector2 End, int Range, FPathingData PathingData, TArray<TEnumAsByte<EPathingRules>> Rules);
	

protected:

	UPROPERTY(Transient)
	bool HasRunBefore = false;

	virtual void ReplaceGridCell(UWorld* World, FIntVector2 Coord);

	virtual void OnConstruction(const FTransform& Transform) override;

	void RotateOffsets(TArray<FIntVector2>& Offsets, EPatternRotation Rotation);

};
