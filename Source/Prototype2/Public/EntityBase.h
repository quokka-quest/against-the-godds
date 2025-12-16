// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "GlobalDataTypeHeader.h"
#include "GameplayAbilityBase.h"
#include "PersistentDataStruct.h"
#include "EntityBase.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AEntityBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetupTurnStart();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnTurnEnd();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EnqueueMovement(FVector StartPos, FVector EndPos);

	UPROPERTY(BlueprintReadWrite, Category="PlayerInfo")
	FIntVector2 PositionCoord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerInfo")
	int MaxMovement;
	UPROPERTY(BlueprintReadWrite, Category="PlayerInfo")
	int AvailableMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	TMap<TSubclassOf<UGameplayAbilityBase>, FDiceFaceLevels> AbilityDiceMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	int MaxAttacks;
	UPROPERTY(BlueprintReadWrite, Category = "PlayerInfo")
	int AvailableAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	TMap<TEnumAsByte<EPatternRotation>, FGridData> EntityRotations;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	FGridData RotationSweep;
	UPROPERTY(BlueprintReadWrite, Category = "PlayerInfo")
	TEnumAsByte<EPatternRotation> FacingDirection;

	void PrintDebugData();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerInfo")
	bool HasEntityDied();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerInfo")
	void OnEntityDeath();

	void SetCharacterData(FPersistentPlayerInfo& Info);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FPathingData GetPathingData();
};
