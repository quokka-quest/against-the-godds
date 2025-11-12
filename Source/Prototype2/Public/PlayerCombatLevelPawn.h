// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GridCell.h"
#include "TileHighlight.h"
#include "GridManager.h"
#include "CombatManager.h"
#include "GlobalDataTypeHeader.h"
#include "PlayerCombatLevelPawn.generated.h"

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

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<ETileSelectionType> TileSelectionType;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ToggleTurnInputMapping(bool EnableInputs);

	UFUNCTION(BlueprintCallable)
	void SetTileSelectionType(ETileSelectionType Type);

protected:
	UFUNCTION(BlueprintCallable)
	void OnTileClick();

	UFUNCTION(BlueprintCallable)
	void OnRotateAttack();

	UPROPERTY(BlueprintReadWrite)
	bool InFreeCamMode = false;

private:
	UPROPERTY()
	APlayerController* PlayerCon;

	UPROPERTY()
	ATileHighlight* TileHighlight;
	
	UPROPERTY()
	AGridCell* HighlightedCell;

	UPROPERTY()
	AGridCell* SelectedCell;

	UPROPERTY()
	TArray<AGridCell*> SelectedStartCells;

	UPROPERTY()
	AGridManager* GridManager;

	UPROPERTY()
	ACombatManager* CombatManager;

	bool IsDisplayingAttack;

	void TryAddTileToSpawnSelection();

	void TryMoveToTile();

	void DisplayPathToTile();

	void DisplayAttackTargetArea();

	void TryAttackTargetTile();

	UFUNCTION(BlueprintCallable)
	bool AttemptToFinishPlayerStartPlacement();

	UFUNCTION()
	void OnPlayerTurnEnd();

	UFUNCTION()
	void OnMoveButtonClicked();

	UFUNCTION()
	void OnAttackButtonClicked();

	void OnClickedOffTileGrid();

	UFUNCTION()
	void OnAttackExecuted();

};
