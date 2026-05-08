// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GridCellParent.h"
#include "GridManagerTool.h"
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

	UFUNCTION(BlueprintCallable)
	void SetSelfTargetInfo(AGridCellParent* Cell, UGameplayAbilityBase* Ability);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDisplayingSelfTargetAbility(UGameplayAbilityBase* Ability);

	UFUNCTION(BlueprintCallable)
	void TryUseSelfTargetAbility();

protected:
	UFUNCTION(BlueprintCallable)
	void OnTileClick();

	UFUNCTION(BlueprintCallable)
	void OnRotateAttack();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetMouseGroundIntersection();

private:
	UPROPERTY()
	APlayerController* PlayerCon;
	
	UPROPERTY()
	AGridCellParent* HighlightedCell;

	UPROPERTY()
	AGridCellParent* SelectedCell;

	UPROPERTY()
	AGridManagerTool* GridManager;

	UPROPERTY()
	ACombatManager* CombatManager;

	UPROPERTY()
	UGameplayAbilityBase* SelfTargetAbilityOnDisplay;

	bool IsDisplayingAttack;
	bool isDisplayingPath;
	bool IsDisplayingSelfTargetArea;

	void TryMoveToTile();

	void DisplayPathToTile();

	void DisplayAttackTargetArea();

	UFUNCTION()
	void OnPlayerTurnEnd();

	UFUNCTION()
	void OnMoveButtonClicked();

	UFUNCTION()
	void OnAttackButtonClicked();

	void OnClickedOffTileGrid();

	UFUNCTION()
	void OnAttackExecuted();
	void TryAttackTargetTile();

	void TurnOffAttackDisplay();
	void TurnOffPathDisplay();

	UFUNCTION()
	FIntVector2 GetCurrentCombatantGridPos();

	void ResetDisplayVariables(ETileSelectionType SelectionType);

};
