// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "GlobalDataTypeHeader.h"
#include "GameplayAbilityBase.h"
#include "PersistentDataStruct.h"
#include "Engine/Texture2D.h"
#include "EntityBase.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AEntityBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEntityBase();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetupTurnStart();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnTurnEnd();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EnqueueMovement(FVector StartPos, FVector EndPos, TSubclassOf<UGameplayAbilityBase> AbilityTrigggered);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EnqueueRotation(float StartYaw, float EndYaw);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ToggleHealthVisibility(bool Enable);

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

	UPROPERTY()
	TMap<TEnumAsByte<EPatternRotation>, float> DirectionYaws;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerInfo")
	UTexture2D* CharacterPortrait;

	void PrintDebugData();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerInfo")
	bool HasEntityDied();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PlayerInfo")
	void OnEntityDeath();

	void SetCharacterData(FPersistentPlayerInfo& Info);

	UFUNCTION(BlueprintCallable, Category = "PlayerInfo")
	void InitialiseStats();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FPathingData GetPathingData();
};
