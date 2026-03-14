// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyEntity.h"
#include "CombatManager.h"
#include "PlayerEntity.h"
#include "Kismet/GameplayStatics.h"

void AEnemyEntity::SetTauntTarget(AEntityBase* EntityTarget, bool SetToEmpty)
{
	if (SetToEmpty) { PriorityTarget = nullptr; return; }
	PriorityTarget = EntityTarget;
}

// This function is the execution sequence of the entire enemy decision-making logic
void AEnemyEntity::TakeTurn()
{
	FindTargetablePlayers();
	UE_LOG(LogTemp, Warning, TEXT("Num of targetable players: %i"), TargetablePlayers.Num())

	// TODO: need to trigger these once on combat start since the info can't change during combat (minor optimisation)
	AnalyseOwnAbilities();
	AnalyseAllPlayerAbilities();

	DetermineMovement();
	UE_LOG(LogTemp, Warning, TEXT("Current coord: %i, %i"), PositionCoord.X, PositionCoord.Y)
	UE_LOG(LogTemp, Warning, TEXT("Highest Score coord: %i, %i, Score: %i"), ActionToTake.Coord.X, ActionToTake.Coord.Y, ActionToTake.Score)
	MoveToTarget();
}

//
void AEnemyEntity::FindTargetablePlayers()
{
	TargetablePlayers.Empty();
	AllAlivePlayers.Empty();

	// if taunted then only add the player that can be attacked
	if (PriorityTarget) TargetablePlayers.Add(PriorityTarget);

	// loop through all entities in the scene. Check if they are a player and alive
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()));
	for (AEntityBase* Entity : CombatManager->Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player || Player->HasEntityDied()) continue; // if the entity is not a player or is dead then do not add it to either set
		if (PriorityTarget) {AllAlivePlayers.Add(Player); continue;} // if there is a taunt active then just add the players to the 'alive player' set
		
		TargetablePlayers.Add(Player);
		AllAlivePlayers.Add(Player);
	}
}

void AEnemyEntity::AnalyseOwnAbilities()
{
	OwnAnalysedAbilities.Empty();
	
	for (UGameplayAbilityBase* Ability : GetAllAbilityInstances())
	{
		FAbilityInfo AbilityInfo = FAbilityInfo(Ability); // TODO: the constructor for this struct extracts all of the abilities information
		OwnAnalysedAbilities.Add(AbilityInfo);
	}
}

void AEnemyEntity::AnalyseAllPlayerAbilities()
{
	PlayerAbilityInfo.Empty();
	
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()));
	for (AEntityBase* Entity : CombatManager->Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player) continue;

		FPlayerAbilityInfo AbilityInfo;
		AbilityInfo.MaxRange = 0;
		AbilityInfo.MaxPotentialDamage = 1; // TODO: need to replace this with an actual method for determining max damage
		AbilityInfo.Effects = FAbilityEffectInfo();
		AbilityInfo.Effects.AppliesBuff = false;
		
		for (UGameplayAbilityBase* Ability : Player->GetAllAbilityInstances())
		{
			if (Ability->Range > AbilityInfo.MaxRange) AbilityInfo.MaxRange = Ability->Range;
			if (Ability->TargetEffects.DoesDamage) AbilityInfo.Effects.DoesDamage = true;
			if (Ability->TargetEffects.AppliesDebuff) AbilityInfo.Effects.AppliesDebuff = true;
		}

		PlayerAbilityInfo.Add(Player, AbilityInfo);
	}
}


void AEnemyEntity::DetermineMovement()
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	TMap<FIntVector2, FPositionInfo> CellActionMap;
	TMap<FIntVector2, int> CellScoreMap;
	
	// Create the FPathFinderInfo struct to use for pathfinding
	FPathfinderInfo PathInfo = FPathfinderInfo();
	PathInfo.StartCoord = PositionCoord;
	PathInfo.Range = AvailableMovement;
	PathInfo.PathingData = GetPathingData();
	PathInfo.Rules.Add(EPathingRules::MustFitOnTarget);
	PathInfo.Rules.Add(EPathingRules::ExcludeOccupiedCells);;
	PathInfo.Rules.Add(EPathingRules::RangeIsAvailableMovement);

	// Get all the cells that can be moved to
	TArray<FIntVector2> CellChoices = GridManager->GetCellsInRange(PathInfo);

	// Determine the score for all available cells
	PathInfo.Rules.Add(EPathingRules::TryPathAroundHazards); // extra rule needed here to avoid lowering the score of cells that could be reached without passing through a hazard if pathed around
	for (FIntVector2 CellChoice : CellChoices)
	{
		PathInfo.TargetCoord = CellChoice;
		TArray<FPathInfo> Path;
		if (!GridManager->PathFindBetweenTwoCoords(Path, PathInfo)) continue; // early continue just in-case something went wrong

		// Run a function to determine the best action to take at the given position
		FPositionInfo PositionInfo = DetermineBestAbilityAtPosition(CellChoice);
		PositionInfo.Path = Path;

		// APPLY BONUS/PENALTY
		
		// Hazard Penalty:
		int TotalHazardsInPath = Path[Path.Num()-1].HazardPenaltyFromStart;
		PositionInfo.Score -= TotalHazardsInPath * 10;

		// Player proximity bonus
		for (AEntityBase* Player : AllAlivePlayers)
		{
			int DistToPlayers = GetDistanceBetweenTwoCoords(CellChoice, Player->PositionCoord);
			PositionInfo.Score += (20-DistToPlayers);
		}

		// if no ability can be used penalty
		if (!PositionInfo.HasTarget)
		{
			PositionInfo.Score -= 1;
			CellScoreMap.Add(CellChoice, PositionInfo.Score);
			CellActionMap.Add(CellChoice, PositionInfo);
			UE_LOG(LogTemp, Warning, TEXT("Coord: %i, %i, Score: %i"), CellChoice.X, CellChoice.Y, PositionInfo.Score)
			continue; // continue to avoid errors from below penalty/bonuses that are dependent on having a target
		}

		CellScoreMap.Add(CellChoice, PositionInfo.Score);
		CellActionMap.Add(CellChoice, PositionInfo);
	}

	// find the action with the highest score
	FIntVector2 BestChoice = FIntVector2(0,0);
	int HighestScore = -99999;
	for (auto& Choice : CellScoreMap)
	{
		if (Choice.Value <= HighestScore) continue;

		HighestScore = Choice.Value;
		BestChoice = Choice.Key;
	}

	ActionToTake = CellActionMap[BestChoice];
}

// This function determines the best ability to use from a given coordinate
// it returns a struct that contains:
// .Coord- the coordinate to move to
// .Score- the weighting the chosen ability has for moving to the chosen coord
// .HasTarget- a bool that is true if an ability to use was identified (some enemies may only have a single attack which would not have a target if it's far away from the players)
// .BestAbility- the ability to use
// .TargetOfAbility- the entity to target with the ability (could be self for a buff or a player for an attack/debuff)
FPositionInfo AEnemyEntity::DetermineBestAbilityAtPosition(FIntVector2 Coord)
{
	TArray<FPositionInfo> EachTargetable;

	// loop through each potential target
	for (AEntityBase* Player : TargetablePlayers)
	{
		TArray<FPositionInfo> EachAbility;
		int DistToTarget = GetDistanceBetweenTwoCoords(Coord, Player->PositionCoord);

		// loop through each available ability to use and determine the best one against the target
		for (FAbilityInfo Info : OwnAnalysedAbilities)
		{
			FPositionInfo PosInfo = FPositionInfo(Coord);
			PosInfo.BestAbility = Info.Ability;
			
			if (!Info.TargetsSelf && !Info.TargetsOpponent) { UE_LOG(LogTemp, Warning, TEXT("AEnemyEntity->DetermineBestAbilityAtPosition: analysed ability has no target")) continue; }

			// if it's a self targeting ability, evaluate its score
			if (Info.TargetsSelf)
			{
				PosInfo.TargetOfAbility = this;
				PosInfo.HasTarget = true;

				// TODO: Add self target score bonuses here
				
				continue;
			}

			// opponent targeting abilities evaluation
			if (Info.Range < DistToTarget) continue; // early continue if the target player is out of range

			PosInfo.TargetOfAbility = Player;
			PosInfo.HasTarget = true;

			// TODO: add attack score bonuses here

			EachAbility.Add(PosInfo);
		}

		FPositionInfo Temp = FPositionInfo(Coord);
		if (!GetHighestScore(EachAbility, Temp)) continue; // if no valid ability was found then move onto the next target

		// otherwise add the chosen ability to the 'EachTargetable' array
		EachTargetable.Add(Temp);
	}
	
	// the default FPositionInfo sets 'HasTarget' to false which can later be checked to see if an ability should be used
	FPositionInfo Temp2 = FPositionInfo(Coord);
	
	// if no attack was identified as possible, then return the default FPositionInfo
	if (EachTargetable.Num() == 0) return Temp2;

	// otherwise return the option with the highest score
	GetHighestScore(EachTargetable, Temp2);
	return Temp2;
}

bool AEnemyEntity::GetHighestScore(TArray<FPositionInfo>& InfoSet, FPositionInfo& OutInfo)
{
	int HighestScore = -99999;
	bool FoundValidScore = false;
	FPositionInfo HighestScoreInfo = FPositionInfo();

	for (FPositionInfo Info : InfoSet)
	{
		if (Info.Score <= HighestScore) continue;
		FoundValidScore = true;
		HighestScoreInfo = Info;
	}

	if (!FoundValidScore) return false;

	OutInfo = HighestScoreInfo;
	return true;
}


// returns the distance between two coordinate (used to find the distance to a targetable player)
int AEnemyEntity::GetDistanceBetweenTwoCoords(FIntVector2 Start, FIntVector2 End)
{
	return (FMath::Abs(Start.X - End.X) + FMath::Abs(Start.Y - End.Y));
}

// NOTE: available movement on enemies is factored into determining where to move to.
// Since no second movement will be taken on their turn, it doesn't need to be tracked during the actual movement
void AEnemyEntity::MoveToTarget()
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));
	int LastIndex = ActionToTake.Path.Num() - 1;
	
	// this for loop queues the needed movement and rotations
	for (int i = 0; i <= LastIndex; i++)
	{
		bool NeedsRot = ActionToTake.Path[i].StartingRot != ActionToTake.Path[i].RotToChangeTo;
		if (NeedsRot) EnqueueRotation(DirectionYaws[ActionToTake.Path[i].StartingRot], DirectionYaws[ActionToTake.Path[i].RotToChangeTo]);

		// get the world positions to move from/to and the cell being moved onto
		FVector StartPos = GridManager->GridCells[ActionToTake.Path[i].StartingCoord]->GetActorLocation();
		FVector EndPos = GridManager->GridCells[ActionToTake.Path[i].CoordToMoveTo]->GetActorLocation();
		AGridCellParent* TargetCell = Cast<AGridCellParent>(GridManager->GridCells[ActionToTake.Path[i].CoordToMoveTo]);

		// Enqueue the movement
		EnqueueMovement(StartPos, EndPos, TargetCell);
	}

	// removes this entity from the cell it started in
	ChangeOccupancy(PositionCoord, false);
	FacingDirection = ActionToTake.Path[LastIndex].RotToChangeTo;

	// set occupancy on the cell that was moved onto
	ChangeOccupancy(ActionToTake.Path[LastIndex].CoordToMoveTo, true);
	PositionCoord = ActionToTake.Path[LastIndex].CoordToMoveTo;
}


void AEnemyEntity::ChangeOccupancy(FIntVector2 Coord, bool SetAsOccupier)
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	for (FIntVector2 Offset : EntityRotations[FacingDirection].GetSelectedCellOffsets())
	{
		FIntVector2 CellCoord = Coord + Offset;
		GridManager->GridCells[CellCoord]->SetOccupier((SetAsOccupier)? this : nullptr);
	}
}
