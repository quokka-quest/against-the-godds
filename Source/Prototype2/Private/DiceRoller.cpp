// Fill out your copyright notice in the Description page of Project Settings.


#include "DiceRoller.h"
#include "Engine/World.h"

// Sets default values
ADiceRoller::ADiceRoller()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bRollComplete = false;
}

FDiceFaceValues ADiceRoller::BlueprintRollDice(FDiceFaceLevels DiceValues)
{
	// WARNING: This is a synchronous function that blocks. Use StartDiceRoll instead!
	bRollComplete = false;
	CurrentResult = FDiceFaceValues();

	// Create a lambda to capture the result
	FOnDiceRollCompleteDelegate Callback;
	Callback.BindLambda([this](FDiceFaceValues Result)
	{
		CurrentResult = Result;
		bRollComplete = true;
	});

	RollDice(DiceValues, Callback);

	// Wait for the roll to complete (with timeout)
	float TimeoutDuration = 10.0f;
	float ElapsedTime = 0.0f;
	float CheckInterval = 0.016f; // ~60 FPS

	UWorld* World = GetWorld();
	if (World)
	{
		while (!bRollComplete && ElapsedTime < TimeoutDuration)
		{
			FPlatformProcess::Sleep(CheckInterval);
			ElapsedTime += CheckInterval;
		}
	}

	if (!bRollComplete)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiceRoller: BlueprintRollDice timed out!"));
	}

	return CurrentResult;
}

void ADiceRoller::StartDiceRoll(FDiceFaceLevels DiceValues)
{
	// Create a callback that broadcasts to Blueprint
	FOnDiceRollCompleteDelegate Callback;
	Callback.BindLambda([this](FDiceFaceValues Result)
	{
		OnDiceRollCompleteBP.Broadcast(Result);
	});

	RollDice(DiceValues, Callback);
}


void ADiceRoller::RollDice(FDiceFaceLevels DiceFaceLevels, FOnDiceRollCompleteDelegate OnComplete)
{
	if (!DiceActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DiceRoller: DiceActorClass is not set!"));
		OnComplete.ExecuteIfBound(FDiceFaceValues());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("DiceRoller: World is null!"));
		OnComplete.ExecuteIfBound(FDiceFaceValues());
		return;
	}

	// Store the callback
	CompletionCallback = OnComplete;

	// Clean up previous dice actor if it exists
	if (CurrentDiceActor)
	{
		CurrentDiceActor->Destroy();
		CurrentDiceActor = nullptr;
	}

	// Spawn the dice actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnPos = GetActorLocation() + SpawnLocation;
	FRotator SpawnRot = FRotator::ZeroRotator;

	CurrentDiceActor = World->SpawnActor<ADiceActor>(DiceActorClass, SpawnPos, SpawnRot, SpawnParams);

	if (!CurrentDiceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("DiceRoller: Failed to spawn DiceActor!"));
		OnComplete.ExecuteIfBound(FDiceFaceValues());
		return;
	}

	CurrentDiceActor->DiceFaces = DiceFaceLevels;
	CurrentDiceActor->OnRollComplete.AddDynamic(this, &ADiceRoller::OnDiceRollCompleteHandler);
	CurrentDiceActor->RollDice();

	// Set up timeout timer
	GetWorld()->GetTimerManager().SetTimer(TimeoutTimerHandle, this, &ADiceRoller::OnRollTimeout, 10.0f, false);
}

void ADiceRoller::OnDiceRollCompleteHandler(FDiceFaceValues Result)
{
	GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    
	if (CurrentDiceActor)
	{
		CurrentDiceActor->OnRollComplete.RemoveDynamic(this, &ADiceRoller::OnDiceRollCompleteHandler);
	}

	CompletionCallback.ExecuteIfBound(Result);
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

void ADiceRoller::OnRollTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("DiceRoller: Dice roll timed out!"));
    
	if (CurrentDiceActor)
	{
		CurrentDiceActor->OnRollComplete.RemoveDynamic(this, &ADiceRoller::OnDiceRollCompleteHandler);
	}
    
	CompletionCallback.ExecuteIfBound(FDiceFaceValues());
}
