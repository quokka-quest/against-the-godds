// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntityBase.h"
#include "EnemyEntity.generated.h"

struct FAbilityInfo
{
	UGameplayAbilityBase* Ability;
	int Range;
	int MaxPotentialDamage;
	FAbilityEffectInfo SelfEffects;
	FAbilityEffectInfo TargetEffects;
};

USTRUCT()
struct FPlayerAbilityInfo
{
	GENERATED_BODY()
	
	int MaxRange;
	int MaxPotentialDamage;
	
	FAbilityEffectInfo Effects;
};

struct FPositionInfo
{
	FIntVector2 Coord;
	int Score;

	bool HasTarget;
	UGameplayAbilityBase* BestAbility;
	AEntityBase* TargetOfAbility;

	FPositionInfo()
	{
		Coord = FIntVector2(0,0);
		Score = 0;
		HasTarget = false;
		BestAbility = nullptr;
		TargetOfAbility = nullptr;
	}

	FPositionInfo(FIntVector2 Coord)
	{
		this->Coord = Coord;
		Score = 0;
		HasTarget = false;
		BestAbility = nullptr;
		TargetOfAbility = nullptr;
	}
};

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AEnemyEntity : public AEntityBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "EnemyLogic")
	void SetTauntTarget(AEntityBase* EntityTarget, bool SetToEmpty);

protected:
	UFUNCTION(BlueprintCallable)
	void TakeTurn();
	
	void FindTargetablePlayers();
	FPositionInfo DetermineBestAbilityAtPosition(FIntVector2 Coord);

	UFUNCTION(BlueprintCallable)
	void DetermineMovement();

	int GetDistanceBetweenTwoCoords(FIntVector2 Start, FIntVector2 End);

	void ChangeOccupancy(FIntVector2 Coord, bool SetAsOccupier);

	UPROPERTY(BlueprintReadWrite, Category="EnemyLogic")
	AEntityBase* PriorityTarget;
	UPROPERTY()
	TSet<AEntityBase*> TargetablePlayers;

	bool GetHighestScore(TArray<FPositionInfo>& InfoSet, FPositionInfo& OutInfo);

	// Map of positions and a struct containing info about the actions to take on them
	TMap<FIntVector2, FPositionInfo> ActionScoring;

	/////////////////////////////////////////////////////////////////////// ability analysis / info
	void AnalyseOwnAbilities();
	TArray<FAbilityInfo> OwnAnalysedAbilities;

	void AnalyseAllPlayerAbilities();
	UPROPERTY()
	TMap<AEntityBase*, FPlayerAbilityInfo> PlayerAbilityInfo;

	// NOTES:
	// need to factor in success change (1/6, 3/6, etc)
	// can calculate ability score as: EffectStrength * SuccessChange * ValueAgainstTarget
	// Value against target determination: 
	
	// BEHAVIOUR PARAMETERS

	// attack related
	const int AttackOpportunityBonus = 1;
	const int KillPotentialBonus = 2;

	// movement related
	const int HazardPenalty = 3;
};
