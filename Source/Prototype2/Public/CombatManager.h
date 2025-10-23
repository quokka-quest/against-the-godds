// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridManager.h"
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

UCLASS()
class PROTOTYPE2_API ACombatManager : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Combat")
	FName TurnEventQueueName;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<AEntityBase*> Combatants;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<FPlayerInitiativeData> DefaultTurnOrder;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<FPlayerInitiativeData> CurrentTurnOrder;
	
	void FinishPlayerLocationPicking(TArray<AGridCell*> &playerStartCells);

	UFUNCTION(BlueprintImplementableEvent)
	void EnableCombatUI();

private:

	UPROPERTY()
	AGridManager* GridManager;

	UFUNCTION()
	void InitialiseCombat();

	void RollDiceForInitiative();

	void SortTurnOrderArray();

	void SpawnEnemies();
	
};
