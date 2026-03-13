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

	// TODO: need to trigger these once on combat start since the info can't change during combat (minor optimisation)
	AnalyseOwnAbilities();
	AnalyseAllPlayerAbilities();

	
}

//
void AEnemyEntity::FindTargetablePlayers()
{
	TargetablePlayers.Empty();

	// if taunted then only add the player that can be attacked
	if (PriorityTarget) {TargetablePlayers.Add(PriorityTarget); return;}
	
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()));
	for (AEntityBase* Entity : CombatManager->Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player) continue;
		if (Player->HasEntityDied()) continue;
		
		TargetablePlayers.Add(Player);
	}
}

void AEnemyEntity::AnalyseOwnAbilities()
{
	OwnAnalysedAbilities.Empty();
	
	for (UGameplayAbilityBase* Ability : GetAllAbilityInstances())
	{
		FAbilityInfo AbilityInfo;
		AbilityInfo.Ability = Ability;
		AbilityInfo.Range = Ability->Range;
		AbilityInfo.MaxPotentialDamage = 1; // TODO: change this to actually find the max damage some how
		AbilityInfo.SelfEffects = Ability->SelfEffects;
		AbilityInfo.TargetEffects = Ability->TargetEffects;
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

		FPositionInfo PositionInfo = DetermineBestAbilityAtPosition(CellChoice);
		// TODO: need to adjust score in the above struct for movement factors
	}






	
	TArray<FPathInfo> PathToTarget;
	// this variable is used to track which index of the 'PathToTarget' array the movement of this entity should stop at
	int TargetPosIndex = -1;
	// the pathing function used above ignores the idea of movement cost so that is checked in this function
	int MoveCost = 0;

	// this for loop establishes how far along the given path this entity can move. The for loop below does the actual movement
	// this is where any and all movement logic for enemy entities should be
	// the above pathfinding function can ignore occupied
	for (int i = 0; i < PathToTarget.Num(); i++)
	{
		// check if the available movement will allow for this tile to be moved to
		int AddMoveCost = GridManager->GridCells[PathToTarget[i].CoordToMoveTo]->MovementCost;
		if (MoveCost + AddMoveCost > MaxMovement) break;

		// if it can then the current tile is set as the new target
		MoveCost += AddMoveCost;
		TargetPosIndex = i; // the target position index changes to be the next cell to move onto
	}

	if (TargetPosIndex < 0) return;

	// apply the movement cost
	AvailableMovement -= MoveCost;

	// this for loop queues the needed movement and rotations
	for (int i = 0; i <= TargetPosIndex; i++)
	{
		bool rot = PathToTarget[i].StartingRot != PathToTarget[i].RotToChangeTo;
		float StartRot = DirectionYaws[PathToTarget[i].StartingRot];
		float EndRot = DirectionYaws[PathToTarget[i].RotToChangeTo];
		if (rot) EnqueueRotation(StartRot, EndRot);

		FVector StartPos = GridManager->GridCells[PathToTarget[i].StartingCoord]->GetActorLocation();
		FVector EndPos = GridManager->GridCells[PathToTarget[i].CoordToMoveTo]->GetActorLocation();

		// check for ability on hazard to trigger when walked on
		AGridCellParent* TargetCell = Cast<AGridCellParent>(GridManager->GridCells[PathToTarget[i].CoordToMoveTo]);
		EnqueueMovement(StartPos, EndPos, TargetCell);
	}

	// removes this entity from the cell it started in
	ChangeOccupancy(PositionCoord, false);
	FacingDirection = PathToTarget[TargetPosIndex].RotToChangeTo;

	// set occupancy on the cell that was moved onto
	ChangeOccupancy(PathToTarget[TargetPosIndex].CoordToMoveTo, true);
	PositionCoord = PathToTarget[TargetPosIndex].CoordToMoveTo;
}

// This function determines the best ability to use from a given coordinate
// it returns a struct that contains:
// .Coord- the coordinate to move to
// .Score- the weighting the chosen ability has for moving to the chosen coord
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
			// TODO: need to fix the fact that this will filter out all self buffs 
			if (Info.Range < DistToTarget) continue;

			FPositionInfo PosInfo = FPositionInfo(Coord);
			PosInfo.BestAbility = Info.Ability;
			PosInfo.TargetOfAbility = Player;
			PosInfo.HasTarget = true;

			//TODO: add score bonuses here

			EachAbility.Add(PosInfo);
		}

		FPositionInfo Temp = FPositionInfo(Coord);
		if (!GetHighestScore(EachAbility, Temp)) continue; // if no valid ability was found then move onto the next target

		// otherwise add the chosen ability to the 'EachTargetable' array
		EachTargetable.Add(Temp);
	}

	FPositionInfo Temp2 = FPositionInfo(Coord);

	// if no attack was identified as possible, then return the default FPositionInfo
	if (EachTargetable.Num() == 0) return Temp2;
	
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

void AEnemyEntity::ChangeOccupancy(FIntVector2 Coord, bool SetAsOccupier)
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	for (FIntVector2 Offset : EntityRotations[FacingDirection].GetSelectedCellOffsets())
	{
		FIntVector2 CellCoord = Coord + Offset;
		GridManager->GridCells[CellCoord]->SetOccupier((SetAsOccupier)? this : nullptr);
	}
}
