// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalDataTypeHeader.generated.h"

// Used for changing the display type of the grid system
UENUM(BlueprintType)
enum EEditorGridDisplayType
{
	Default,
	PlayerSpawnTile,
	HazardTile,
	EnemySpawnTile
};

// Used for determining which material to set on a tile
UENUM(BlueprintType)
enum ETileMaterial
{
	Base,
	Target,
	Highlighted
};

// Used for telling the AttackTargetAreas which attack pattern to use
UENUM(BlueprintType)
enum EAttackPattern
{
	SingleTarget,
	Plus,
	CentredLine3X1,
	EndLine3X1
};

// Used in the player combat level pawn
// Determines the functionality of clicking on a tile
UENUM(BlueprintType)
enum ETileSelectionType
{
	SpawnSelection,
	Movement,
	Attack,
	None
};

UENUM(BlueprintType)
enum EAttackRules
{
	DisplayAsPathToTarget,
	UserMustFitOnTarget,
	StraightLineOnly
};

UENUM(BlueprintType)
enum class ETargetType : uint8
{
	TT_Character UMETA(DisplayName = "Character"),
	TT_Tile UMETA(DisplayName = "Tile")
};

USTRUCT(BlueprintType)
struct FDiceFaceValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PrimaryLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SecondaryLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TertiaryLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int FourthLevel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int FifthLevel = 0;
};

USTRUCT(BlueprintType)
struct FDiceFaceLevels 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDiceFaceValues> FaceArray;

	FDiceFaceLevels() {
		FaceArray.SetNum(0);
	}

	FDiceFaceValues GetFaceValues(int FaceNum) 
	{
		if (abs(FaceNum) < FaceArray.Num()) return FaceArray[abs(FaceNum)];
		
		return FaceArray[0];
	}
};

USTRUCT(BlueprintType)
struct FAbilityEffectInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DoesDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AppliesBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AppliesDebuff;
};
