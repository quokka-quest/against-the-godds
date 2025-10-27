// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "PathFinder.h"
#include "GridManager.generated.h"

UENUM(BlueprintType)
enum EEditorGridDisplayType
{
	Default,
	PlayerSpawnTile,
	HazardTile,
	EnemySpawnTile
};

UENUM(BlueprintType)
enum ETileMaterial
{
	Base,
	Target,
	Highlighted
};

UCLASS()
class PROTOTYPE2_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TEnumAsByte<EEditorGridDisplayType> GridDisplayType;
	
	UPROPERTY(BlueprintReadWrite, Category = "Grid")
	TMap<FIntVector, AGridCell*> GridCells;

private:
	UPROPERTY()
	UMaterialInterface* DefaultMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TileRed.M_TileRed"));
	UPROPERTY()
	UMaterialInterface* TargetMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TargetTile.M_TargetTile"));
	UPROPERTY()
	UMaterialInterface* HighlightedMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TileGreen.M_TileGreen"));
	UPROPERTY()
	UMaterialInterface* PathMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TileGold.M_TileGold"));

	UPROPERTY()
	APathFinder* PathFinder;
	
	void InitialiseGridManagement();

	TArray<FIntVector> NeighbourOffsets = {
		FIntVector(1,0,0),
		FIntVector(-1,0,0),
		FIntVector(1,0,1),
		FIntVector(1,0,-1),
		FIntVector(-1,0,1),
		FIntVector(-1,0,-1),
		FIntVector(0,1,0),
		FIntVector(0,-1,0),
		FIntVector(0,1,1),
		FIntVector(0,1,-1),
		FIntVector(0,-1,1),
		FIntVector(0,-1,-1)
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:

	UFUNCTION(BlueprintCallable)
	void ChangeAllTilesDisplay(EEditorGridDisplayType DisplayType);

	UFUNCTION()
	void ChangeTilesMaterial(AGridCell* Tile, ETileMaterial Material);

	UFUNCTION(BlueprintCallable)
	void DisplayWalkableTiles(FIntVector CurrentCellCoord, int AvailableMovement);

	UFUNCTION(BlueprintCallable)
	void ResetWalkableTiles();

	UFUNCTION(BlueprintCallable)
	TArray<FIntVector> DisplayTilePath(FIntVector StartCoord, FIntVector EndCoord);

};
