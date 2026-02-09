// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridManagerClass.h"
#include "GlobalDataTypeHeader.h"
#include "GridCellParent.h"
#include "GridOutlineActor.h"
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

	UFUNCTION(BlueprintCallable)
	void ResetHighlights();

	UFUNCTION(BlueprintCallable)
	void DisplayWalkableCells(FIntVector2 Start, int AvailableMovement, FPathingData PathData);

	UFUNCTION(BlueprintCallable)
	TArray<FPathInfo> DisplayCellPath(FIntVector2 StartCoord, FIntVector2 EndCoord, FPathingData PathData);

	UFUNCTION(BlueprintCallable)
	void DisplayCellsInAttackRange(FIntVector2 Start, int Range, FPathingData PathData, TArray<TEnumAsByte<EAttackRules>>& Rules);

	UFUNCTION(BlueprintCallable)
	TArray<FIntVector2> DisplayAttackPattern(FIntVector2 TargetCoord, FGridData Pattern, EPatternRotation Rotation, FPathingData PathData, TArray<TEnumAsByte<EAttackRules>>& Rules);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FIntVector2> GetPlayerSpawnCells();

	void SetHighlightPosition(FIntVector2 CellCoord);
	void SetHighlightVisibility(bool IsVisible);
	void SetHighlightRotation(float Rotation);

	UFUNCTION(BlueprintCallable)
	void ChangeHighlightMesh(FGridData& HighlightData);

protected:
	
	UPROPERTY()
	TArray<AActor*> DirectionIndicators;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	TSubclassOf<AActor> ArrowIndicator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	TSubclassOf<AActor> AxisIndicator;
	UPROPERTY()
	bool DisplayArrows;
	UPROPERTY()
	AActor* AxisActorRef;
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "GridDisplay")
	void ToggleDirectionIndicators();
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "GridDisplay")
	void ToggleAxisIndicator();

	UPROPERTY()
	AGridOutlineActor* OutlineActor;
	UPROPERTY()
	AGridOutlineActor* AreaOutlineActor;
	UPROPERTY()
	AGridOutlineActor* PathAndAttackOutlineActor;
	UPROPERTY()
	AGridOutlineActor* HighlightOutlineActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	UMaterialInterface* GridMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	UMaterialInterface* AreaMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	UMaterialInterface* PathMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridDisplay")
	UMaterialInterface* HighlightMaterial;
	
	virtual void ReplaceGridCell(UWorld* World, FIntVector2 Coord) override;

	virtual void BeginPlay() override;

	bool DoesPatternFitOnCell(FIntVector2 CellCoord, TArray<FIntVector2>& Offsets, FPathingData& PathData);
	
};
