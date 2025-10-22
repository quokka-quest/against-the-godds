// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.generated.h"

UCLASS()
class PROTOTYPE2_API AGridCell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridCell();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FIntVector GridCellCoord;

private:
	float cellSize = 100.0f;
	float cellHeight = 50.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
