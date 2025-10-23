// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridManager.h"
#include "CombatManager.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API ACombatManager : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Combat")
	FName TurnEventQueueName;

private:
	void EnablePlayerLocationPicking();

	UPROPERTY()
	AGridManager* GridManager;
	
};
