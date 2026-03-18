// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridCellBase.h"
#include "EnemyEntity.h"
#include "GridData.h"
#include "GridCellParent.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AGridCellParent : public AGridCellBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	bool IsPlayerSpawnCell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	bool IsEnviroHazardCell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	bool IsEnemySpawnCell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	TSubclassOf<AEnemyEntity> EnemyToSpawn;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellProperties")
	TEnumAsByte<EPatternRotation> SpawnedEntityRotation;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CellProperties")
	void ToggleNiagraForCellEffect(bool Enable);
	UPROPERTY(BlueprintReadWrite, Category = "CellProperties")
	bool HasEffect;
	UPROPERTY(BlueprintReadWrite, Category = "CellProperties")
	TSubclassOf<UGameplayAbilityBase> TemporaryCellEffect;
	UPROPERTY(BlueprintReadWrite, Category = "CellProperties")
	int NumOfRepeats;
	
};
