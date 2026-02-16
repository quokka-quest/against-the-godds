// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCellBase.generated.h"

UCLASS(Abstract)
class GRIDSYSTEM_API AGridCellBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridCellBase();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:
	//////////////////////////////////////////////////////////////////////////// Editor exposed properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CellInfo")
	float ZOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CellInfo")
	bool BlockPositiveX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CellInfo")
	bool BlockNegativeX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CellInfo")
	bool BlockPositiveY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CellInfo")
	bool BlockNegativeY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CellInfo")
	int MovementCost;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category="CellInfo")
	FIntVector2 CellCoordinate;

	//////////////////////////////////////////////////////////////////////////// Other properties
	UPROPERTY(BlueprintReadWrite, Category="CellInfo")
	bool IsOccupied;
	UPROPERTY(BlueprintReadWrite, Category="CellInfo")
	AActor* OccupyingActor;
	UPROPERTY(BlueprintReadWrite, Category="CellInfo")
	bool IsWalkable;
	UPROPERTY(BlueprintReadWrite, Category="CellInfo")
	bool IsAttackable;

	//////////////////////////////////////////////////////////////////////////// Getters and setters

	// sets the occupying actor. Can pass in a nullptr to set occupancy to false
	UFUNCTION(BlueprintCallable, Category="CellInfo")
	void SetOccupier(AActor* Occupant) {OccupyingActor = Occupant; IsOccupied = (Occupant)? true: false;}

};
