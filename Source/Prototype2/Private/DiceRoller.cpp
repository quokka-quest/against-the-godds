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

	FVector SpawnPos;
	FRotator SpawnRot;
	
	if (SpawnPoint)
	{
		SpawnPos = SpawnPoint->GetComponentLocation();
		SpawnRot = SpawnPoint->GetComponentRotation();
	}
	else
	{
		// Fallback if SpawnLoc arrow component not found
		SpawnPos = GetActorLocation() + FVector(0.0f, 0.0f, 500.0f);
		SpawnRot = FRotator::ZeroRotator;
		UE_LOG(LogTemp, Warning, TEXT("DiceRoller: Using fallback spawn location"));
	}

	CurrentDiceActor = World->SpawnActor<ADiceActor>(DiceActorClass, SpawnPos, SpawnRot, SpawnParams);

	if (!CurrentDiceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("DiceRoller: Failed to spawn DiceActor!"));
		OnComplete.ExecuteIfBound(FDiceFaceValues());
		return;
	}

	CurrentDiceActor->DiceFaces = DiceFaceLevels;
	CurrentDiceActor->UpdateDiceFaces();
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
		CurrentDiceActor->Destroy();
		CurrentDiceActor = nullptr;
	}

	CompletionCallback.ExecuteIfBound(Result);
}

// Called when the game starts or when spawned
void ADiceRoller::BeginPlay()
{
	Super::BeginPlay();
	
	// Find the arrow component named "SpawnLoc"
	TArray<UArrowComponent*> ArrowComponents;
	GetComponents<UArrowComponent>(ArrowComponents);
	
	for (UArrowComponent* Arrow : ArrowComponents)
	{
		if (Arrow && Arrow->GetName() == TEXT("SpawnLoc"))
		{
			SpawnPoint = Arrow;
			UE_LOG(LogTemp, Log, TEXT("DiceRoller: Found SpawnLoc arrow component"));
			break;
		}
	}
	
	if (!SpawnPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiceRoller: Could not find arrow component named 'SpawnLoc'"));
	}
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
