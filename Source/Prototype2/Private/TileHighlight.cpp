// Fill out your copyright notice in the Description page of Project Settings.


#include "TileHighlight.h"

// Sets default values
ATileHighlight::ATileHighlight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HeightOffset = FVector(0,0,10);
}

// Called when the game starts or when spawned
void ATileHighlight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATileHighlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATileHighlight::MoveToPosition(FVector TargetPos)
{
	SetActorLocation(TargetPos + HeightOffset);
}

