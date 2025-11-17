// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalDataTypeHeader.h"
#include "DiceActor.generated.h"

UCLASS()
class PROTOTYPE2_API ADiceActor : public AActor
{
	GENERATED_BODY()
    
public:    
	ADiceActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	UStaticMeshComponent* DiceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FDiceFaceLevels DiceFaces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float RollForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float RollTorque = 1000.0f;

	UFUNCTION(BlueprintCallable, Category = "Dice")
	void RollDice();

	UFUNCTION(BlueprintCallable, Category = "Dice")
	FDiceFaceValues GetResultingFace();

protected:
	virtual void BeginPlay() override;

private:
	int GetTopFaceIndex();
	bool bIsRolling = false;
	FTimerHandle StabilityCheckTimer;
	void CheckIfStable();
};
