// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GridCell.h"
#include "TileHighlight.h"
#include "GridManager.h"
#include "PlayerCombatLevelPawn.generated.h"

UENUM(BlueprintType)
enum ETileSelectionType
{
	SpawnSelection,
	Movement
};

UCLASS()
class PROTOTYPE2_API APlayerCombatLevelPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCombatLevelPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY()
	TEnumAsByte<ETileSelectionType> TileSelectionType;

private:
	UPROPERTY()
	APlayerController* PlayerCon;

	UPROPERTY()
	ATileHighlight* TileHighlight;
	
	UPROPERTY()
	AGridCell* HighlightedCell;

	UPROPERTY()
	TArray<AGridCell*> SelectedStartCells;

	UPROPERTY()
	AGridManager* GridManager;

	UFUNCTION(BlueprintCallable)
	void OnTileClick();

	void TryAddTileToSpawnSelection();

};
