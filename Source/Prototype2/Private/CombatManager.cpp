// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"

#include "PlayerCombatLevelPawn.h"
#include "PlayerEntity.h"
#include "Kismet/GameplayStatics.h"


// Sets references to needed manager classes on start
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::PlayerSpawnTile);

	CurrentCombatantTurnIndex = 0;
	SetAttackRotation(EPatternRotation::R0);
}

// Called when the player locks in there start location choices
// spawns the player characters on the chosen tiles
void ACombatManager::FinishPlayerLocationPicking(TArray<AGridCellParent*> &playerStartCells)
{
	for (AGridCellParent*& Cell : playerStartCells)
	{
		FVector Loc = Cell->GetActorLocation();
		FRotator Rot = Cell->GetActorRotation();
		FTransform Transform = FTransform(Rot, Loc);
		AEntityBase* player = GetWorld()->SpawnActor<AEntityBase>(PlayerClass, Transform);
		Combatants.Add(player);
		player->PositionCoord = Cell->CellCoordinate;
		Cell->SetOccupier(player);
	}

	OnPlayerSpawnLocsPicked();
}

// spawns enemies on the relevant tiles and populates the turn order array
void ACombatManager::SpawnEnemies()
{
	for (auto& Cell: GridManager->GridCells)
	{
		AGridCellParent* Value = Cast<AGridCellParent>(Cell.Value);
		if (Value->IsEnemySpawnCell)
		{
			FTransform form = Value->GetTransform();
			AEnemyEntity* enemy = GetWorld()->SpawnActor<AEnemyEntity>(Value->EnemyToSpawn, form);
			Combatants.Add(enemy);
			enemy->PositionCoord = Value->CellCoordinate;
			Value->SetOccupier(enemy);
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

// called to start a new turn
void ACombatManager::StartCurrentTurn()
{
	CurrentTurnCombatant = CurrentTurnOrder[CurrentCombatantTurnIndex].Entity;
	AEnemyEntity* EnemyRef = Cast<AEnemyEntity>(CurrentTurnCombatant);
	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant);

	CurrentTurnCombatant->AvailableMovement = CurrentTurnCombatant->MaxMovement;
	CurrentTurnCombatant->AvailableAttacks = CurrentTurnCombatant->MaxAttacks;

	RoundHasEnded = false;
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

// called by Entities on the end of their turn
// manages end of turn logic and starts the next turn
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
	BlueprintEndTurnEvents(); // calls a function defined in the game-mode blueprint that will transition UI and start the next turn
}

void ACombatManager::IncrementTurnIndex()
{
	CurrentCombatantTurnIndex += 1;
	CurrentCombatantTurnIndex %= CurrentTurnOrder.Num();
	if (CurrentCombatantTurnIndex == 0) RoundHasEnded = true;
	if (CurrentTurnOrder[CurrentCombatantTurnIndex].Entity->HasEntityDied()) IncrementTurnIndex();
}

// only used for player movement. Enemy movement is handled in the EnemyEntity class
// moves a player from there current position to the target position
// checks for valid locations are done before this function is called so they aren't needed here
void ACombatManager::MoveCurrentCombatant(FIntVector2 TargetPos)
{
	GridManager->GridCells[CurrentTurnCombatant->PositionCoord]->SetOccupier(nullptr);
	GridManager->GridCells[TargetPos]->SetOccupier(CurrentTurnCombatant);

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
void ACombatManager::DisplayPathForCurrentCombatant(FIntVector2 TargetPos)
{
	FIntVector2 StartPos = CurrentTurnCombatant->PositionCoord;
	UE_LOG(LogTemp, Warning, TEXT("Start pos: %i, %i"), StartPos.X, StartPos.Y)
	UE_LOG(LogTemp, Warning, TEXT("Target pos: %i, %i"), TargetPos.X, TargetPos.Y)
	PathForCombatantToFollow = GridManager->DisplayCellPath(StartPos, TargetPos);
}

// displays the movement options for the current combatant
void ACombatManager::DisplayCurrentCombatantsMovement()
{
	GridManager->ResetWalkableAndAttackableOnAllCells();
	GridManager->ChangeAllTilesDisplay(Default);
	GridManager->DisplayWalkableCells(CurrentTurnCombatant->PositionCoord, CurrentTurnCombatant->AvailableMovement);
}

// displays all the tiles that the player can target
void ACombatManager::DisplayAttackRange(int Range) 
{
	AttackRange = Range;
	GridManager->ResetWalkableAndAttackableOnAllCells();
	GridManager->ChangeAllTilesDisplay(Default);
	GridManager->DisplayCellsInAttackRange(CurrentTurnCombatant->PositionCoord, Range);
}

// displays the attack area and stores the targeted tiles with element 0 being the targeted tile and the rest are the additional area
void ACombatManager::DisplayAttackPattern(FIntVector2 TargetCoord)
{
	DisplayAttackRange(AttackRange);
	AreaOfAttackEffect = GridManager->DisplayAttackPattern(TargetCoord, AttackPattern, AttackRotation);
}

void ACombatManager::DisplayAttackInformation(TSubclassOf<UGameplayAbilityBase> Ability, FDiceFaceLevels DiceLevels, int Range, FGridData Pattern)
{
	AbilityToUse = Ability;
	AttackPattern = Pattern;
	DisplayAttackRange(Range);
}

void ACombatManager::ExecuteAttackOnTarget()
{
	TArray<AActor*> TargetActors;

	for (int i = 0; i < AreaOfAttackEffect.Num(); i++)
	{
		if (!GridManager->GridCells.Contains(AreaOfAttackEffect[i])) continue; // continue if cell coordinate is invalid
		if (!GridManager->GridCells[AreaOfAttackEffect[i]]->IsOccupied) continue; // continue if cell is not occupied
		if (!GridManager->GridCells[AreaOfAttackEffect[i]]->OccupyingActor)
			{ UE_LOG(LogTemp, Warning, TEXT("CombatManager->ExecuteAttackOnTarget() found an occupied cell with a null occupant")) continue;} // continue if cell is occupied but the entity is null
		
		TargetActors.Add(Cast<AActor>(GridManager->GridCells[AreaOfAttackEffect[i]]->OccupyingActor));
	}

	CurrentTurnCombatant->AvailableAttacks--;
	CurrentTurnCombatant->ActivateAbilityWithTargets(AbilityToUse, TargetActors);
	
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::Default);
	OnAttackExecuted.Broadcast();
}

void ACombatManager::OnEntityDeath(AEntityBase* DeadEntity)
{
	// mark the tile the entity was on as empty
	GridManager->GridCells[DeadEntity->PositionCoord]->SetOccupier(nullptr);
}

void ACombatManager::EnemySetAttackInfo(TSubclassOf<UGameplayAbilityBase> Ability, FDiceFaceLevels DiceLevels, FGridData Pattern, FIntVector2 TargetPos, EPatternRotation Rotation)
{
	AbilityToUse = Ability;
	AttackPattern = Pattern;
	AreaOfAttackEffect = GridManager->GetCellsInAttackArea(TargetPos, Pattern, Rotation);
}



/////////////////////////////////////////////////////////////////////////// Blueprint friendly Getters and setters:

EPatternRotation ACombatManager::GetAttackRotation()
{
	return AttackRotation;
}

void ACombatManager::SetAttackRotation(EPatternRotation Rotation)
{
	AttackRotation = Rotation;
}

AEntityBase* ACombatManager::GetCurrentCombatant()
{
	return CurrentTurnCombatant;
}

FName ACombatManager::GetTurnQueueName()
{
	return TurnEventQueueName;
}

bool ACombatManager::HasRoundEnded()
{
	return RoundHasEnded;
}


/////////////////////////////////////////////////////////////////////////// Blueprint friendly delegate broadcasts:

void ACombatManager::BroadcastOnMoveClickedEvent()
{
	OnMoveButtonClicked.Broadcast();
}

void ACombatManager::BroadcastOnAttackClickedEvent() 
{
	OnAttackButtonClicked.Broadcast();
}