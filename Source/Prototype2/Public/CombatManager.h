// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridManager.h"
#include "PlayerEntity.h"
#include "CombatManager.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FPlayerInitiativeData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	AEntityBase* Entity;
	UPROPERTY(BlueprintReadWrite)
	int Initiative;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerTurnEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveButtonClicked);

UCLASS()
class PROTOTYPE2_API ACombatManager : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, category = "Combat")
	FOnPlayerTurnEnd OnPlayerTurnEnd;

	UPROPERTY(BlueprintAssignable, category = "Combat")
	FOnMoveButtonClicked OnMoveButtonClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Combat")
	FName TurnEventQueueName;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<AEntityBase*> Combatants;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<FPlayerInitiativeData> DefaultTurnOrder;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<FPlayerInitiativeData> CurrentTurnOrder;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	AEntityBase* CurrentTurnCombatant;
	
	void FinishPlayerLocationPicking(TArray<AGridCell*> &playerStartCells);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerSpawnLocsPicked();
	
	void MoveCurrentCombatant(FIntVector TargetPos);

	void DisplayPathForCurrentCombatant(FIntVector TargetPos);

	UFUNCTION(BlueprintCallable)
	void DisplayCurrentCombatantsMovement();

	UFUNCTION(BlueprintCallable)
	void BroadcastOnMoveClickedEvent();

protected:
	UPROPERTY(BlueprintReadWrite)
	AGridManager* GridManager;

	UPROPERTY(BlueprintReadWrite)
	int CurrentCombatantTurnIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, category = "Combat")
	TSubclassOf<APlayerEntity> PlayerClass;

	UFUNCTION(BlueprintCallable)
	void RollDiceForInitiative();

	void SortTurnOrderArray();

	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();

	UFUNCTION(BlueprintCallable)
	void StartCurrentTurn();

	UFUNCTION(BlueprintCallable)
	void EndCurrentTurn();

	void IncrementTurnIndex();

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintEndTurnEvents();
	
	
};
