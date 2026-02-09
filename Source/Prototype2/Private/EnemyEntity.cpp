// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyEntity.h"
#include "CombatManager.h"
#include "Kismet/GameplayStatics.h"

void AEnemyEntity::DeterminePlayerTarget()
{
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()));
	
	int ShortestDist = 100000;
	APlayerEntity* ClosestPlayer = nullptr;
	
	for (AEntityBase* Entity : CombatManager->Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player) continue;
		if (Player->HasEntityDied()) continue;

		FIntVector2 PlayerPos = Player->PositionCoord;
		int XDist = abs(PlayerPos.X - PositionCoord.X);
		int YDist = abs(PlayerPos.Y - PositionCoord.Y);
		int totalDist = XDist + YDist;

		if (totalDist >= ShortestDist) continue;

		ShortestDist = totalDist;
		ClosestPlayer = Player;
	}

	PlayerTarget = ClosestPlayer;
}

void AEnemyEntity::DetermineMovement()
{
	// if the player target is invalid something has gone wrong so do not move
	if (!PlayerTarget) return;

	// find the longest attack range of this enemy
	int MaxRange = 0;
	for (UGameplayAbilityBase* Ability: GetAllAbilityInstances())
	{
		if (Ability->Range > MaxRange) MaxRange = Ability->Range;
	}
	// if the target is in the attack range there's no need to move
	if (IsTargetInAttackRange(MaxRange)) return;

	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	TArray<TEnumAsByte<EAttackRules>> Rules; // empty but needed to use the below function
	TArray<FPathInfo> PathToTarget = GridManager->GetPathToPointInRangeOfTarget(PositionCoord, PlayerTarget->PositionCoord, MaxRange, GetPathingData(), Rules);

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
		TSubclassOf<UGameplayAbilityBase> CellAbility = nullptr;
		if (Cast<AGridCellParent>(GridManager->GridCells[PathToTarget[i].CoordToMoveTo])->TemporaryCellEffect)
		{
			CellAbility = Cast<AGridCellParent>(GridManager->GridCells[PathToTarget[i].CoordToMoveTo])->TemporaryCellEffect;
		}
		
		EnqueueMovement(StartPos, EndPos, CellAbility);
	}

	// removes this entity from the cell it started in
	ChangeOccupancy(PositionCoord, false);
	FacingDirection = PathToTarget[TargetPosIndex].RotToChangeTo;

	// set occupancy on the cell that was moved onto
	ChangeOccupancy(PathToTarget[TargetPosIndex].CoordToMoveTo, true);
	PositionCoord = PathToTarget[TargetPosIndex].CoordToMoveTo;
}

// TODO: improve attack selection
bool AEnemyEntity::DetermineAttack()
{
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
	int DistToTarget = abs(PositionCoord.X - PlayerTarget->PositionCoord.X) + abs(PositionCoord.Y - PlayerTarget->PositionCoord.Y);

	for (UGameplayAbilityBase* Ability: GetAllAbilityInstances())
	{
		if (DistToTarget > Ability->Range) continue;
		CombatManager->EnemySetAttackInfo(Ability, PlayerTarget->PositionCoord, EPatternRotation::R0);
		CombatManager->ExecuteAttackOnTarget();
		return true;
	}

	return false;
}

bool AEnemyEntity::IsTargetInAttackRange(int Range)
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	TArray<TEnumAsByte<EAttackRules>> Rules; // empty but needed to use below function
	TArray<FIntVector2> AttackableTiles = GridManager->GetCellsInAttackRange(PositionCoord, Range, GetPathingData(), Rules);
	for (int i = 0; i < AttackableTiles.Num(); i++)
	{
		if (!GridManager->GridCells[AttackableTiles[i]]->IsOccupied) continue;
		if (GridManager->GridCells[AttackableTiles[i]]->OccupyingActor == PlayerTarget) return true;
	}
	
	return false;
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
