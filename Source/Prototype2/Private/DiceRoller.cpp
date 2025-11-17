// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceRoller.h"

// Sets default values
ADiceRoller::ADiceRoller()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

FDiceFaceValues ADiceRoller::RollDice(FDiceFaceLevels)
{
	return FDiceFaceValues();
}

// Called when the game starts or when spawned
void ADiceRoller::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADiceRoller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

