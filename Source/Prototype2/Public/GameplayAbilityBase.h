// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilityBase.generated.h"

UENUM()
enum class ETargetType : uint8
{
	TT_Character UMETA(DisplayName = "Character"),
	TT_Tile UMETA(DisplayName = "Tile")
};

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual float CalculateDamageWithMods(float BaseDamage);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual int RollDice();

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	ETargetType TargetType;
};
