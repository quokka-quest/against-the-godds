// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"

#include "PlayerCombatLevelPawn.h"
#include "PlayerEntity.h"
#include "Kismet/GameplayStatics.h"


// Sets references to needed manager classes on start
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::PlayerSpawnTile);

	CurrentCombatantTurnIndex = 0;
	SetAttackRotation(EAttackRotation::R0);
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
		AEntityBase* player = GetWorld()->SpawnActor<AEntityBase>(PlayerClass, Transform);
		Combatants.Add(player);
		player->PositionCoord = Cell->GridCellCoord;
		Cell->IsOccupied = true;
	}

	OnPlayerSpawnLocsPicked();
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
			enemy->PositionCoord = Value->GridCellCoord;
			Value->IsOccupied = true;
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

// Sets random initiative values for all combatants then sorts the array
void ACombatManager::RollDiceForInitiative()
{
	for (int i = 0; i < DefaultTurnOrder.Num(); i++)
	{
		DefaultTurnOrder[i].Initiative = FMath::RandRange(1, 20);
	}

	SortTurnOrderArray();
}

// a simple bubble sort for sorting the turn order array
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

void ACombatManager::StartCurrentTurn()
{
	CurrentTurnCombatant = CurrentTurnOrder[CurrentCombatantTurnIndex].Entity;
	AEnemyEntity* EnemyRef = Cast<AEnemyEntity>(CurrentTurnCombatant);
	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant);

	CurrentTurnCombatant->AvailableMovement = CurrentTurnCombatant->MaxMovement;
	
	CurrentTurnCombatant->SetupTurnStart();

	// for additional Enemy specific logic
	if (EnemyRef)
	{
		
	}
	// for additional player specific logic
	if (PlayerRef)
	{
		APlayerCombatLevelPawn* Player = Cast<APlayerCombatLevelPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerCombatLevelPawn::StaticClass()));
		if (!Player) {UE_LOG(LogTemp, Error, TEXT("Player pawn doesn't exist")) return;}
		Player->ToggleTurnInputMapping(true);
	}
}

void ACombatManager::EndCurrentTurn()
{
	AEnemyEntity* EnemyRef = Cast<AEnemyEntity>(CurrentTurnCombatant);
	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant);

	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::Default);
	
	// for additional Enemy specific logic
	if (EnemyRef)
	{
		
	}
	// for additional player specific logic
	if (PlayerRef)
	{
		APlayerCombatLevelPawn* Player = Cast<APlayerCombatLevelPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerCombatLevelPawn::StaticClass()));
		if (!Player) {UE_LOG(LogTemp, Error, TEXT("Player pawn doesn't exist")) return;}
		Player->ToggleTurnInputMapping(false);
		OnPlayerTurnEnd.Broadcast();
	}
	
	IncrementTurnIndex();
	BlueprintEndTurnEvents();
}

void ACombatManager::IncrementTurnIndex()
{
	CurrentCombatantTurnIndex += 1;
	CurrentCombatantTurnIndex %= CurrentTurnOrder.Num();
}

void ACombatManager::MoveCurrentCombatant(FIntVector TargetPos)
{
	GridManager->GridCells[CurrentTurnCombatant->PositionCoord]->IsOccupied = false;
	GridManager->GridCells[CurrentTurnCombatant->PositionCoord]->OccupyingEntity = nullptr;
	GridManager->GridCells[TargetPos]->IsOccupied = true;
	GridManager->GridCells[TargetPos]->OccupyingEntity = CurrentTurnCombatant;

	for (int i = PathForCombatantToFollow.Num()-1; i > 0; i--)
	{
		FVector StartPos = GridManager->GridCells[PathForCombatantToFollow[i]]->GetActorLocation();
		FVector EndPos = GridManager->GridCells[PathForCombatantToFollow[i-1]]->GetActorLocation();
		CurrentTurnCombatant->EnqueueMovement(StartPos, EndPos);
		CurrentTurnCombatant->AvailableMovement -= GridManager->GridCells[PathForCombatantToFollow[i-1]]->MovementCost;
	}

	CurrentTurnCombatant->PositionCoord = TargetPos;
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::Default);
}

// displays the path to be taken by a combatant if they were to move to the target position
void ACombatManager::DisplayPathForCurrentCombatant(FIntVector TargetPos)
{
	FIntVector StartPos = CurrentTurnCombatant->PositionCoord;
	PathForCombatantToFollow = GridManager->DisplayTilePath(StartPos, TargetPos);
}

// displays the movement options for the current combatant
void ACombatManager::DisplayCurrentCombatantsMovement()
{
	GridManager->ResetTilesWalkAndAttackBooleans();
	GridManager->DisplayWalkableTiles(CurrentTurnCombatant->PositionCoord, CurrentTurnCombatant->AvailableMovement);
}

void ACombatManager::BroadcastOnMoveClickedEvent()
{
	OnMoveButtonClicked.Broadcast();
}

void ACombatManager::BroadcastOnAttackClickedEvent() 
{
	OnAttackButtonClicked.Broadcast();
}

AEntityBase* ACombatManager::GetCurrentCombatant()
{
	return CurrentTurnCombatant;
}

void ACombatManager::DisplayAttackRange(int Range) 
{
	AttackRange = Range;
	GridManager->ResetTilesWalkAndAttackBooleans();
	GridManager->DisplayTilesInAttackRange(CurrentTurnCombatant->PositionCoord, Range);
}

void ACombatManager::DisplayAttackPattern(FIntVector TargetCoord)
{
	DisplayAttackRange(AttackRange);
	AreaOfAttackEffect = GridManager->DisplayAttackPattern(TargetCoord, AttackPattern, AttackRotation);
}

void ACombatManager::SetAttackPatternToUse(EAttackPattern Pattern)
{
	AttackPattern = Pattern;
}

void ACombatManager::SetAttackRotation(EAttackRotation Rotation)
{
	AttackRotation = Rotation;
}

EAttackRotation ACombatManager::GetAttackRotation()
{
	return AttackRotation;
}

void ACombatManager::DisplayAttackInformation() {

}
