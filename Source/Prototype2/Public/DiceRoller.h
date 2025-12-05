// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "GlobalDataTypeHeader.h"
#include "DiceActor.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiceRoller.generated.h"

DECLARE_DELEGATE_OneParam(FOnDiceRollCompleteDelegate, FDiceFaceValues);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceRollCompleteBP, FDiceFaceValues, Result);

UCLASS()
class PROTOTYPE2_API ADiceRoller : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADiceRoller();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TSubclassOf<ADiceActor> DiceActorClass;

	// Blueprint event that fires when dice roll completes
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceRollCompleteBP OnDiceRollCompleteBP;

	// Start a dice roll - result will be broadcast via OnDiceRollCompleteBP
	UFUNCTION(BlueprintCallable, Category = "Dice")
	void StartDiceRoll(FDiceFaceLevels DiceValues);
	virtual void RollDice(FDiceFaceLevels DiceFaceLevels, FOnDiceRollCompleteDelegate OnComplete);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FOnDiceRollCompleteDelegate CompletionCallback;
	FTimerHandle TimeoutTimerHandle;

	void OnRollTimeout();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UArrowComponent* SpawnPoint;
	
	UPROPERTY()
	ADiceActor* CurrentDiceActor;
	
	FDiceFaceValues CurrentResult;
	bool bRollComplete;

	UFUNCTION()
	void OnDiceRollCompleteHandler(FDiceFaceValues Result);
};
