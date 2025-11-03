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
	HorizontalLine3X1,
	VerticalLine3X1
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