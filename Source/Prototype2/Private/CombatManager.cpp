// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"
#include "PlayerEntity.h"
#include "Kismet/GameplayStatics.h"

// Sets references to needed manager classes on start
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::PlayerSpawnTile);
}

// Called when the player locks in there start location choices
// spawns the player characters on the chosen tiles
void ACombatManager::FinishPlayerLocationPicking(TArray<AGridCell*> &playerStartCells)
{
	for (AGridCell*& Cell : playerStartCells)
	{
		FVector Loc = Cell->GetActorLocation();
		FRotator Rot = Cell->GetActorRotation();
		FTransform Transform = FTransform(Rot, Loc);
		APlayerEntity* player = GetWorld()->SpawnActor<APlayerEntity>(APlayerEntity::StaticClass(), Transform);
		Combatants.Add(player);
	}

	InitialiseCombat();
}

void ACombatManager::InitialiseCombat()
{
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::Default);
	
	SpawnEnemies();
	RollDiceForInitiative();
	SortTurnOrderArray();
	EnableCombatUI();
}

// spawns enemies on the relevant tiles and populates the turn order array
void ACombatManager::SpawnEnemies()
{
	for (auto& Cell: GridManager->GridCells)
	{
		AGridCell* Value = Cell.Value;
		if (Value->IsEnemySpawnTile)
		{
			FTransform form = Value->GetTransform();
			AEnemyEntity* enemy = GetWorld()->SpawnActor<AEnemyEntity>(Value->EnemyToSpawn, form);
			Combatants.Add(enemy);
		}
	}

	// populate the turn order array
	for (AEntityBase* Entity : Combatants)
	{
		FPlayerInitiativeData Data = FPlayerInitiativeData();
		Data.Entity = Entity;
		Data.Initiative = 0;
		DefaultTurnOrder.Add(Data);
	}
}


void ACombatManager::RollDiceForInitiative()
{
	for (int i = 0; i < DefaultTurnOrder.Num(); i++)
	{
		DefaultTurnOrder[i].Initiative = FMath::RandRange(1, 20);
	}
}

// a simple bubble sort for sorting the turn order
void ACombatManager::SortTurnOrderArray()
{
	bool Swapped = false;

	for (int i = 0; i < DefaultTurnOrder.Num() - 1; i++)
	{
		Swapped = false;
		for (int j = 0; j < DefaultTurnOrder.Num()-1; j++)
		{
			if (DefaultTurnOrder[j].Initiative > DefaultTurnOrder[j+1].Initiative)
			{
				Swap<FPlayerInitiativeData>(DefaultTurnOrder[j], DefaultTurnOrder[j+1]);
				Swapped = true;
			}
		}
		if (!Swapped) break;
	}

	CurrentTurnOrder = DefaultTurnOrder;
}


