// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"

#include "PlayerCombatLevelPawn.h"
#include "GameManager.h"
#include "GlobalDataTypeHeader.h"
#include "GameplayAbilityBase.h"
#include "Kismet/GameplayStatics.h"


// Sets references to needed manager classes on start
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));
	GridManager->ResetHighlights();
	
	CurrentCombatantTurnIndex = 0;
	SetAttackRotation(EPatternRotation::R0);
}

// Called when the player locks in there start location choices
// spawns the player characters on the chosen tiles
void ACombatManager::SpawnPlayerCharacters()
{
	TArray<FIntVector2> SpawnableCells = GridManager->GetPlayerSpawnCells();
	// < 3 comes from the max number of player characters being 3
	if (SpawnableCells.Num() < 3) {UE_LOG(LogTemp, Error, TEXT("CombatManager->SpawnPlayerCharacters(): insufficient spawnable cells")) return;}

	UGameManager* GameInst =  Cast<UGameManager>(GetGameInstance());
	if (!GameInst) {UE_LOG(LogTemp, Error, TEXT("CombatManager->SpawnPlayerCharacters(): GameInst failed to cast")) return;}

	// fallback for testing the game without having to go through all the game menus
	if (GameInst->CharacterInfo.IsEmpty())
	{
		const int RandIndex = FMath::RandRange(0, SpawnableCells.Num() - 1);
		FIntVector2 Coord = SpawnableCells[RandIndex];
		SpawnEntity(PlayerClass, Coord);
		
		return;
	}
	
	for (auto& Player : GameInst->CharacterInfo)
	{
		const int RandIndex = FMath::RandRange(0, SpawnableCells.Num() - 1);
		FIntVector2 Coord = SpawnableCells[RandIndex];
		SpawnableCells.Remove(Coord);

		APlayerEntity* APlayer = Cast<APlayerEntity>(SpawnEntity(Player.Key, Coord));

		APlayer->SetCharacterData(Player.Value);
	}
}

// spawns enemies on the relevant tiles and populates the turn order array
void ACombatManager::SpawnEnemies()
{
	for (auto& Cell: GridManager->GridCells)
	{
		AGridCellParent* Value = Cast<AGridCellParent>(Cell.Value);
		if (Value->IsEnemySpawnCell)
		{
			Cast<AEnemyEntity>(SpawnEntity(Value->EnemyToSpawn, Cell.Key));
		}
	}
}

AEntityBase* ACombatManager::SpawnEntity(TSubclassOf<AEntityBase> Entity, FIntVector2 SpawnCoord)
{
	EPatternRotation spawnRot = Cast<AGridCellParent>(GetCell(SpawnCoord))->SpawnedEntityRotation;
	float ZRot = (spawnRot == R0)? 90: (spawnRot == R90)? 0: (spawnRot == R180)? -90: 180;
	FRotator Rot = FRotator(0, ZRot, 0);
	FVector Pos = GetCell(SpawnCoord)->GetActorLocation();
	FTransform Form = FTransform(Rot, Pos);

	AEntityBase* EntityRef = GetWorld()->SpawnActor<AEntityBase>(Entity, Form);
	EntityRef->FacingDirection = spawnRot;
	Combatants.Add(EntityRef);
	EntityRef->PositionCoord = SpawnCoord;

	SetCellsOccupier(EntityRef, SpawnCoord, true);

	return EntityRef;
}


// Sets random initiative values for all combatants then sorts the array
void ACombatManager::RollDiceForInitiative()
{
	// populate the turn order array
	for (AEntityBase* Entity : Combatants)
	{
		FPlayerInitiativeData Data = FPlayerInitiativeData();
		Data.Entity = Entity;
		Data.Initiative = 0;
		DefaultTurnOrder.Add(Data);
	}

	// roll for initiative
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
	// set the reference to the entity taking its turn
	CurrentTurnCombatant = CurrentTurnOrder[CurrentCombatantTurnIndex].Entity;
	AEnemyEntity* EnemyRef = Cast<AEnemyEntity>(CurrentTurnCombatant); // used to determine if it's an enemy entity
	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant); // used to determine if it's a player entity

	// reset the available movement and attacks
	CurrentTurnCombatant->AvailableMovement = CurrentTurnCombatant->MaxMovement;
	CurrentTurnCombatant->AvailableAttacks = CurrentTurnCombatant->MaxAttacks;

	// reset the RoundHasEnded boolean
	RoundHasEnded = false;

	// tell the combatant to initialise there turn (Players do initialisation, Enemies execute there full turn here)
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

// called by Entities on the end of their turn (Entities will perform their own end of turn logic BEFORE calling this)
// manages end of turn logic and starts the next turn
void ACombatManager::EndCurrentTurn()
{
	AEnemyEntity* EnemyRef = Cast<AEnemyEntity>(CurrentTurnCombatant);
	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant);

	if (HaveEnemiesWon() || HavePlayersWon()) {UE_LOG(LogTemp, Warning, TEXT("CombatManager->EndCurrentTurn: someone has won")) return;}
	
	GridManager->ResetHighlights();
	
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

	FGridData DefaultHighlightSize = FGridData();
	GridManager->ChangeHighlightMesh(DefaultHighlightSize);
	
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
	if (PathForCombatantToFollow.IsEmpty()) return;

	for (int i = 0; i < PathForCombatantToFollow.Num(); i++)
	{
		float StartRot = CurrentTurnCombatant->DirectionYaws[PathForCombatantToFollow[i].StartingRot];
		float EndRot = CurrentTurnCombatant->DirectionYaws[PathForCombatantToFollow[i].RotToChangeTo];
		bool NeedRot = PathForCombatantToFollow[i].StartingRot != PathForCombatantToFollow[i].RotToChangeTo;
		if (NeedRot) CurrentTurnCombatant->EnqueueRotation(StartRot, EndRot);
		
		FVector StartPos = GetCell(PathForCombatantToFollow[i].StartingCoord)->GetActorLocation();
		FVector EndPos = GetCell(PathForCombatantToFollow[i].CoordToMoveTo)->GetActorLocation();

		// get target cell
		AGridCellParent* TargetCell = Cast<AGridCellParent>(GetCell(PathForCombatantToFollow[i].CoordToMoveTo));
		
		// enqueue movement animation
		CurrentTurnCombatant->EnqueueMovement(StartPos, EndPos, TargetCell);
		CurrentTurnCombatant->AvailableMovement -= GetCell(PathForCombatantToFollow[i].CoordToMoveTo)->MovementCost;
	}

	// remove from old cell and change facing direction
	SetCellsOccupier(CurrentTurnCombatant, CurrentTurnCombatant->PositionCoord, false);
	CurrentTurnCombatant->FacingDirection = PathForCombatantToFollow[PathForCombatantToFollow.Num()-1].RotToChangeTo;

	// set as occupier of new cell
	SetCellsOccupier(CurrentTurnCombatant, TargetPos, true);
	CurrentTurnCombatant->PositionCoord = TargetPos;

	// reset highlight display
	GridManager->ResetHighlights();

	FGridData DefaultHighlightSize = FGridData();
	GridManager->ChangeHighlightMesh(DefaultHighlightSize);
}

// displays the path to be taken by a combatant if they were to move to the target position
void ACombatManager::DisplayPathForCurrentCombatant(FIntVector2 TargetPos)
{
	FPathfinderInfo PathingInfo = FPathfinderInfo();
	PathingInfo.StartCoord = CurrentTurnCombatant->PositionCoord;
	PathingInfo.TargetCoord = TargetPos;
	PathingInfo.Range = CurrentTurnCombatant->AvailableMovement;
	PathingInfo.PathingData = CurrentTurnCombatant->GetPathingData();

	// All rules to apply for player pathfinding
	PathingInfo.Rules.Add(EPathingRules::ExcludeOccupiedCells);
	PathingInfo.Rules.Add(EPathingRules::MustFitOnTarget);
	PathingInfo.Rules.Add(EPathingRules::RangeIsAvailableMovement);
	PathingInfo.Rules.Add(EPathingRules::TryPathAroundHazards);

	// if the path is not valid then clear 'PathForCombatantToFollow' and return
	if (!GridManager->PathFindBetweenTwoCoords(PathForCombatantToFollow, PathingInfo)) {PathForCombatantToFollow.Empty(); return;}

	// otherwise, tell the grid manager to display the path for the current turn entity
	GridManager->DisplayCellPath(PathForCombatantToFollow, CurrentTurnCombatant);
}

// Defines the rules to be used for displaying the current entity's movement range then calls another function to display that range
void ACombatManager::DisplayCurrentCombatantsMovement()
{
	TArray<TEnumAsByte<EPathingRules>> Rules;
	Rules.Add(EPathingRules::ExcludeOccupiedCells);
	Rules.Add(EPathingRules::RangeIsAvailableMovement);
	Rules.Add(EPathingRules::MustFitOnTarget);
	
	DisplayRangeOutline(CurrentTurnCombatant->PositionCoord, CurrentTurnCombatant->AvailableMovement, CurrentTurnCombatant->GetPathingData(), Rules);
}

// Defines the rules to be used for displaying the attack's range then calls another function to display that range
void ACombatManager::DisplayAttackRange(int Range)
{
	TArray<TEnumAsByte<EPathingRules>> Rules;
	if (AbilityRef->TargetingRules.Contains(EAttackRules::UserMustFitOnTarget)) Rules.Add(EPathingRules::MustFitOnTarget);
	if (AbilityRef->TargetingRules.Contains(EAttackRules::StraightLineOnly)) Rules.Add(EPathingRules::StraightLine);
	
	DisplayRangeOutline(CurrentTurnCombatant->PositionCoord, Range, CurrentTurnCombatant->GetPathingData(), Rules);
}

// calls a series of gridManager functions to display the outline of a given range
// used by 'DisplayAttackRange' and 'DisplayCurrentCombatantsMovement'
void ACombatManager::DisplayRangeOutline(FIntVector2 Origin, int Range, FPathingData PathData, TArray<TEnumAsByte<EPathingRules>>& Rules)
{
	GridManager->ResetWalkableAndAttackableOnAllCells();
	GridManager->ResetHighlights();
	GridManager->DisplayCellsInRange(Origin, Range, PathData, Rules);
}


// displays the attack pattern and stores the targeted cells with element 0 being the targeted cell and the rest are the additional area
void ACombatManager::DisplayAttackPattern(FIntVector2 TargetCoord)
{
	AreaOfAttackEffect = GridManager->DisplayAttackPattern(TargetCoord, AbilityRef->Pattern, AttackRotation, CurrentTurnCombatant->GetPathingData(), AbilityRef->TargetingRules);
}

void ACombatManager::DisplayAttackInformation(UGameplayAbilityBase* AbilityInstance)
{
	AbilityRef = AbilityInstance;
	DisplayAttackRange(AbilityInstance->Range);

	if (AbilityInstance->DisplayPatternForTargeting) GridManager->ChangeHighlightMesh(AbilityInstance->Pattern);
}

void ACombatManager::ExecuteAttackOnTarget()
{
	TArray<AActor*> TargetActors;
	for (int i = 0; i < AreaOfAttackEffect.Num(); i++)
	{
		if (!DoesCoordExist(AreaOfAttackEffect[i])) continue; // continue if cell coordinate is invalid

		// if the ability is targeting the cells then add the cell to the target array and continue
		if (AbilityRef->TargetType == ETargetType::TT_Tile)
		{
			TargetActors.Add(GetCell(AreaOfAttackEffect[i]));
			continue;
		}
		
		if (!GetCell(AreaOfAttackEffect[i])->IsOccupied) continue; // continue if cell is not occupied
		if (!GetCell(AreaOfAttackEffect[i])->OccupyingActor) { UE_LOG(LogTemp, Warning, TEXT("CombatManager->ExecuteAttackOnTarget() found an occupied cell with a null occupant")) continue;} // continue if cell is occupied but the entity is null
		
		TargetActors.Add(Cast<AActor>(GetCell(AreaOfAttackEffect[i])->OccupyingActor));
	}

	CurrentTurnCombatant->AvailableAttacks--;
	CurrentTurnCombatant->ActivateAbilityWithTargets(AbilityRef->GetClass(), TargetActors);

	FGridData defaultHighlight = FGridData();
	GridManager->ResetHighlights();
	GridManager->ChangeHighlightMesh(defaultHighlight);
	SetAttackRotation(R0);
	
	OnAttackExecuted.Broadcast();
}

void ACombatManager::OnEntityDeath(AEntityBase* DeadEntity)
{
	// mark the tile the entity was on as empty
	SetCellsOccupier(DeadEntity, DeadEntity->PositionCoord, false);

	if (HavePlayersWon()) OnPlayersWin();
	if (HaveEnemiesWon()) OnPlayersLost();
}

void ACombatManager::EnemySetAttackInfo(UGameplayAbilityBase* AbilityInstance, FIntVector2 TargetPos, EPatternRotation Rotation)
{
	AbilityRef = AbilityInstance;
	AreaOfAttackEffect = GridManager->GetPatternCellsFromTarget(TargetPos, AbilityInstance->Pattern, Rotation);
}

bool ACombatManager::HavePlayersWon()
{
	for (AEntityBase* Entity : Combatants)
	{
		AEnemyEntity* Enemy = Cast<AEnemyEntity>(Entity);
		if (!Enemy) continue;
		if (!Cast<AEnemyEntity>(Entity)->HasEntityDied()) return false;
	}
	return true;
}

bool ACombatManager::HaveEnemiesWon()
{
	for (AEntityBase* Entity : Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player) continue;
		if (!Player->HasEntityDied()) return false;
	}
	return true;
}

void ACombatManager::RemoveDeadPlayers()
{
	for (AEntityBase* Entity : Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player) continue;
		if (Player->HasEntityDied()) {UE_LOG(LogTemp, Warning, TEXT("Player dead on start")) Player->OnEntityDeath();}
		else {UE_LOG(LogTemp, Warning, TEXT("Player alive on start"))}
	}
}

void ACombatManager::SetCellsOccupier(AEntityBase* Entity, FIntVector2 Coord, bool SetAsOccupied)
{
	for (FIntVector2 Offset: Entity->EntityRotations[Entity->FacingDirection].GetSelectedCellOffsets())
	{
		FIntVector2 CellCoord = Coord + Offset;
		GetCell(CellCoord)->SetOccupier((SetAsOccupied)? Entity: nullptr);
	}
}

void ACombatManager::ChangeEntitysOccupancy(AEntityBase* Entity, bool ClearOccupancy)
{
	if (ClearOccupancy) SetCellsOccupier(Entity, Entity->PositionCoord, false);
	else SetCellsOccupier(Entity, Entity->PositionCoord, true);
}

void ACombatManager::ChangeEntityLocation(AEntityBase* Entity, FIntVector2 NewCoord)
{
	SetCellsOccupier(Entity, Entity->PositionCoord, false);
	SetCellsOccupier(Entity, NewCoord, true);
	Entity->PositionCoord = NewCoord;
}

void ACombatManager::SwapEntitiesLocations(AEntityBase* Entity, AEntityBase* TargetEntity)
{
	FIntVector2 originCoord = Entity->PositionCoord;
	FIntVector2 targetCoord = TargetEntity->PositionCoord;

	SetCellsOccupier(Entity, originCoord, false);
	SetCellsOccupier(TargetEntity, targetCoord, false);

	SetCellsOccupier(Entity, targetCoord, true);
	SetCellsOccupier(TargetEntity, originCoord, true);
}

// checks if a given grid pattern is available for placement on a given target entity
// (mostly used to check if the PhysTank can grow to 2x2)
bool ACombatManager::ValidateFullGridPatternAtTarget(AEntityBase* TargetEntity, FGridData Pattern)
{
	TArray<FIntVector2> Offsets = Pattern.GetSelectedCellOffsets();
	FIntVector2 TargetCoord = TargetEntity->PositionCoord;

	for (FIntVector2 Offset: Offsets)
	{
		FIntVector2 CellCoord = TargetCoord + Offset;
		if (!DoesCoordExist(CellCoord)) return false;
		if (GetCell(CellCoord)->IsOccupied && GetCell(CellCoord)->OccupyingActor != TargetEntity) return false;
	}
	
	return true;
}

bool ACombatManager::ValidateFullGridPatternAtCoord(FIntVector2 const TargetCoord, FGridData Pattern)
{
	TArray<FIntVector2> Offsets = Pattern.GetSelectedCellOffsets();

	for (FIntVector2 Offset: Offsets)
	{
		FIntVector2 CellCoord = TargetCoord + Offset;
		if (!DoesCoordExist(CellCoord)) return false;
		if (GetCell(CellCoord)->IsOccupied) return false;
	}
	
	return true;
}

void ACombatManager::AbilityBasedMovement(AEntityBase* EntityToMove, FIntVector2 TargetCoord, float Speed, bool IsKnockback)
{
	FPathfinderInfo PathingInfo = FPathfinderInfo();
	PathingInfo.StartCoord = EntityToMove->PositionCoord;
	PathingInfo.TargetCoord = TargetCoord;
	PathingInfo.Range = 1000;
	PathingInfo.PathingData = EntityToMove->GetPathingData();
	
	if (!GridManager->PathFindBetweenTwoCoords(PathForCombatantToFollow, PathingInfo))
		{UE_LOG(LogTemp, Error, TEXT("CombatManager.cpp->AbilityBasedMovement: path was empty")) return;}

	for (int i = 0; i < PathForCombatantToFollow.Num(); i++)
	{
		float StartRot = EntityToMove->DirectionYaws[PathForCombatantToFollow[i].StartingRot];
		float EndRot = EntityToMove->DirectionYaws[PathForCombatantToFollow[i].RotToChangeTo];
		bool NeedRot = PathForCombatantToFollow[i].StartingRot != PathForCombatantToFollow[i].RotToChangeTo;
		if (NeedRot && !IsKnockback) EntityToMove->EnqueueRotation(StartRot, EndRot);
		
		FVector StartPos = GetCell(PathForCombatantToFollow[i].StartingCoord)->GetActorLocation();
		FVector EndPos = GetCell(PathForCombatantToFollow[i].CoordToMoveTo)->GetActorLocation();

		// get target cell
		AGridCellParent* TargetCell = Cast<AGridCellParent>(GetCell(PathForCombatantToFollow[i].CoordToMoveTo));
		
		// enqueue movement animation
		EntityToMove->EnqueueMovement(StartPos, EndPos, TargetCell);
	}

	// remove from old cell and change facing direction
	SetCellsOccupier(EntityToMove, EntityToMove->PositionCoord, false);
	EntityToMove->FacingDirection = PathForCombatantToFollow[PathForCombatantToFollow.Num()-1].RotToChangeTo;

	// set as occupier of new cell
	SetCellsOccupier(EntityToMove, TargetCoord, true);
	EntityToMove->PositionCoord = TargetCoord;

	// reset highlight display
	GridManager->ResetHighlights();

	FGridData DefaultHighlightSize = FGridData();
	GridManager->ChangeHighlightMesh(DefaultHighlightSize);
}

bool ACombatManager::ApplyKnockback(AEntityBase* Entity, FGridData KnockbackData) 
{
	TArray<FIntVector2> Offsets = KnockbackData.GetSelectedCellOffsets();
	FIntVector2 StartCoord = Entity->PositionCoord;

	for (FIntVector2 Offset : Offsets) 
	{
		FIntVector2 Coord = StartCoord + Offset;
		if (Coord == StartCoord) continue;
		if (!DoesCoordExist(Coord)) continue;

		FPathfinderInfo PathingInfo = FPathfinderInfo();
		PathingInfo.StartCoord = Entity->PositionCoord;
		PathingInfo.TargetCoord = Coord;
		PathingInfo.Range = 1000;
		PathingInfo.PathingData = Entity->GetPathingData();
		
		TArray<FPathInfo> Path;
		if (GridManager->PathFindBetweenTwoCoords(Path, PathingInfo))
			{ UE_LOG(LogTemp, Warning, TEXT("Path To Coord: %i, %i failed"), Coord.X, Coord.Y) continue; }

		AbilityBasedMovement(Entity, Coord, 0.0f, true);
		return true;
	}

	return false;
}

void ACombatManager::EnemyAbilityUse(UGameplayAbilityBase* Ability, FIntVector2 TargetCoord)
{
	AreaOfAttackEffect = GridManager->GetPatternCellsFromTarget(TargetCoord, Ability->Pattern, R0);
	AbilityRef = Ability;
	
	TArray<AActor*> TargetActors;
	for (int i = 0; i < AreaOfAttackEffect.Num(); i++)
	{
		if (!DoesCoordExist(AreaOfAttackEffect[i])) continue; // continue if cell coordinate is invalid

		// if the ability is targeting the cells then add the cell to the target array and continue
		if (AbilityRef->TargetType == ETargetType::TT_Tile)
		{
			TargetActors.Add(GetCell(AreaOfAttackEffect[i]));
			continue;
		}
		
		if (!GetCell(AreaOfAttackEffect[i])->IsOccupied) continue; // continue if cell is not occupied
		if (!GetCell(AreaOfAttackEffect[i])->OccupyingActor) { UE_LOG(LogTemp, Warning, TEXT("CombatManager->ExecuteAttackOnTarget() found an occupied cell with a null occupant")) continue;} // continue if cell is occupied but the entity is null
		
		TargetActors.Add(GetCell(AreaOfAttackEffect[i])->OccupyingActor);
	}

	CurrentTurnCombatant->AvailableAttacks--;
	CurrentTurnCombatant->ActivateAbilityWithTargets(AbilityRef->GetClass(), TargetActors);

	FGridData defaultHighlight = FGridData();
	GridManager->ResetHighlights();
	GridManager->ChangeHighlightMesh(defaultHighlight);
	SetAttackRotation(R0);
	
	OnAttackExecuted.Broadcast();
}


/////////////////////////////////////////////////////////////////////////// Grid Information access
bool ACombatManager::DoesCoordExist(FIntVector2 Coord)
{
	return GridManager->GridCells.Contains(Coord);
}

AGridCellBase* ACombatManager::GetCell(FIntVector2 Coord)
{
	return GridManager->GridCells[Coord];
}

float ACombatManager::GetGridWidth() 
{
	return (float)GridManager->GridData.Columns;
}

/////////////////////////////////////////////////////////////////////////// Blueprint friendly Getters and setters:

EPatternRotation ACombatManager::GetAttackRotation()
{
	return AttackRotation;
}

void ACombatManager::SetAttackRotation(EPatternRotation Rotation)
{
	AttackRotation = Rotation;
	float rot = (AttackRotation == R0)? 0.0f : (AttackRotation == R90)? -90.0f : (AttackRotation == R180)? 180.0f : 90.0f;
	GridManager->SetHighlightRotation(rot);
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

	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant);
	if (!PlayerRef) return;
	if (PlayerRef->AvailableMovement == 0) return;
	
	GridManager->ChangeHighlightMesh(PlayerRef->RotationSweep);
}

void ACombatManager::BroadcastOnAttackClickedEvent() 
{
	OnAttackButtonClicked.Broadcast();

	APlayerEntity* PlayerRef = Cast<APlayerEntity>(CurrentTurnCombatant);
	if (!PlayerRef) return;

	if (AbilityRef->TargetingRules.Contains(EAttackRules::UserMustFitOnTarget)) GridManager->ChangeHighlightMesh(PlayerRef->RotationSweep);
	else { FGridData Default = FGridData(); GridManager->ChangeHighlightMesh(Default); }
}