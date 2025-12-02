// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridManagerTool.h"
#include "GridCellParent.h"
#include "PlayerEntity.h"
#include "GlobalDataTypeHeader.h"
#include "GameplayAbilityBase.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackExecuted);

UCLASS()
class PROTOTYPE2_API ACombatManager : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// delegates
	UPROPERTY(BlueprintAssignable, category = "Combat")
	FOnPlayerTurnEnd OnPlayerTurnEnd;

	UPROPERTY(BlueprintAssignable, category = "Combat")
	FOnMoveButtonClicked OnMoveButtonClicked;

	UPROPERTY(BlueprintAssignable, category = "Combat")
	FOnAttackButtonClicked OnAttackButtonClicked;

	FOnAttackExecuted OnAttackExecuted;

	// properties
	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<AEntityBase*> Combatants;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<FPlayerInitiativeData> DefaultTurnOrder;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TArray<FPlayerInitiativeData> CurrentTurnOrder;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	AEntityBase* CurrentTurnCombatant;

	UPROPERTY(BlueprintReadWrite, category = "Combat")
	TSubclassOf<UGameplayAbilityBase> AbilityToUse;

	// functions
	void MoveCurrentCombatant(FIntVector2 TargetPos);

	void DisplayPathForCurrentCombatant(FIntVector2 TargetPos);
	
	UFUNCTION(BlueprintCallable)
	void DisplayCurrentCombatantsMovement();

	UFUNCTION(BlueprintCallable)
	void DisplayAttackRange(int Range);

	void DisplayAttackPattern(FIntVector2 TargetCoord);

	UFUNCTION(BlueprintCallable)
	void DisplayAttackInformation(TSubclassOf<UGameplayAbilityBase> Ability, FDiceFaceLevels DiceLevels, int Range, FGridData Pattern);

	void ExecuteAttackOnTarget();

	UFUNCTION(BlueprintCallable)
	void OnEntityDeath(AEntityBase* DeadEntity);

	void EnemySetAttackInfo(TSubclassOf<UGameplayAbilityBase> Ability, FDiceFaceLevels DiceLevels, FGridData Pattern, FIntVector2 TargetPos, EPatternRotation Rotation);

	////////////////////////////////////////////////// blueprint getters and setters:
	UFUNCTION(BlueprintCallable)
	void SetAttackRotation(EPatternRotation Rotation);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	EPatternRotation GetAttackRotation();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AEntityBase* GetCurrentCombatant();

	UFUNCTION(blueprintCallable, BlueprintPure)
	FName GetTurnQueueName();

	////////////////////////////////////////////////// blueprint delegate broadcasts
	UFUNCTION(BlueprintCallable)
	void BroadcastOnMoveClickedEvent();

	UFUNCTION(BlueprintCallable)
	void BroadcastOnAttackClickedEvent();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Combat")
	FName TurnEventQueueName;
	
	UPROPERTY(BlueprintReadWrite)
	AGridManagerTool* GridManager;

	UPROPERTY(BlueprintReadWrite)
	int CurrentCombatantTurnIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, category = "Combat")
	TSubclassOf<APlayerEntity> PlayerClass;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool RoundHasEnded;

	UFUNCTION(BlueprintCallable)
	void RollDiceForInitiative();

	void SortTurnOrderArray();
	
	UFUNCTION(BlueprintCallable)
	void SpawnPlayerCharacters();
	
	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();

	UFUNCTION(BlueprintCallable)
	void StartCurrentTurn();

	UFUNCTION(BlueprintCallable)
	void EndCurrentTurn();

	void IncrementTurnIndex();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasRoundEnded();

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintEndTurnEvents();

	TArray<FIntVector2> PathForCombatantToFollow;

	TArray<FIntVector2> AreaOfAttackEffect;

	int AttackRange;
	FGridData AttackPattern;
	EPatternRotation AttackRotation;
	
};
