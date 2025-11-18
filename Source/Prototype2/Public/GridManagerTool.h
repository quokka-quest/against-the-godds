// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridManagerClass.h"
#include "GlobalDataTypeHeader.h"
#include "GridCellParent.h"
#include "GridManagerTool.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AGridManagerTool : public AGridManagerClass
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	TEnumAsByte<EEditorGridDisplayType> GridDisplayType;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "GridDisplay")
	void UpdateDisplay();

	UFUNCTION(BlueprintCallable)
	void ChangeAllTilesDisplay(EEditorGridDisplayType DisplayType);

	UFUNCTION()
	void ChangeCellsMaterial(AGridCellParent* Tile, ETileMaterial Material);

	UFUNCTION(BlueprintCallable)
	void DisplayWalkableCells(FIntVector2 Start, int AvailableMovement);

	UFUNCTION(BlueprintCallable)
	TArray<FIntVector2> DisplayCellPath(FIntVector2 StartCoord, FIntVector2 EndCoord);

	UFUNCTION(BlueprintCallable)
	void DisplayCellsInAttackRange(FIntVector2 Start, int Range);

	UFUNCTION(BlueprintCallable)
	TArray<FIntVector2> DisplayAttackPattern(FIntVector2 TargetCoord, FGridData Pattern, EPatternRotation Rotation);

protected:
	UPROPERTY()
	UMaterialInterface* DefaultMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TileRed.M_TileRed"));
	UPROPERTY()
	UMaterialInterface* TargetMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TargetTile.M_TargetTile"));
	UPROPERTY()
	UMaterialInterface* HighlightedMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TileGreen.M_TileGreen"));
	UPROPERTY()
	UMaterialInterface* PathMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TileGold.M_TileGold"));

	virtual void ReplaceGridCell(UWorld* World, FIntVector2 Coord) override;
	
};
