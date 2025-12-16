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

	TArray<FPathInfo> PathToTarget = GridManager->GetPathToPointInRangeOfTarget(PositionCoord, PlayerTarget->PositionCoord, MaxRange, GetPathingData());

	// establish the first tile in the path
	// pathing gives the path in reverse to the last index is the start tile
	// the -2 gives the first tile to move to
	int TargetPosIndex = PathToTarget.Num() - 2;
	if (TargetPosIndex < 0) return;

	// pathing will give a path from the enemy to the player (ignoring occupancy) so when they are adjacent it will still return a valid path
	// this if statement prevents the enemy from walking onto the player's cell when they are directly next to each other
	if (GridManager->GridCells[PathToTarget[TargetPosIndex].NextCellCoord]->IsOccupied) return;
	// If the movement cost is greater than the available movement of this enemy entity, then movement can't be done
	int MoveCost = GridManager->GridCells[PathToTarget[TargetPosIndex].NextCellCoord]->MovementCost;
	if (MoveCost > MaxMovement) return;

	// -3 is done here because the .Num()-2 tile is done above to establish if movement is possible
	for (int i = PathToTarget.Num()-3; i >= 0; i--)
	{
		// breaks the loop if the tile has an entity on it (prevents movement onto an occupied tile)
		if (GridManager->GridCells[PathToTarget[i].NextCellCoord]->IsOccupied) break;

		// check if the available movement will allow for this tile to be moved to
		int AddMoveCost = GridManager->GridCells[PathToTarget[i].NextCellCoord]->MovementCost;
		if (MoveCost + AddMoveCost > MaxMovement) break;

		// if it can then the current tile is set as the new target
		MoveCost += AddMoveCost;
		TargetPosIndex = i;
	}

	AvailableMovement -= MoveCost;
	for (int i = PathToTarget.Num()-1; i > TargetPosIndex; i--)
	{
		FVector StartPos = GridManager->GridCells[PathToTarget[i].NextCellCoord]->GetActorLocation();
		FVector EndPos = GridManager->GridCells[PathToTarget[i-1].NextCellCoord]->GetActorLocation();
		EnqueueMovement(StartPos, EndPos);
	}

	// removes this entity from the cell it started in
	GridManager->GridCells[PositionCoord]->IsOccupied = false;
	GridManager->GridCells[PositionCoord]->OccupyingActor = nullptr;

	// tells the cell it moved to that it is occupied
	PositionCoord = PathToTarget[TargetPosIndex].NextCellCoord;
	GridManager->GridCells[PositionCoord]->IsOccupied = true;
	GridManager->GridCells[PositionCoord]->OccupyingActor = Cast<AEntityBase>(this);
}

bool AEnemyEntity::DetermineAttack()
{
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
	
	TSubclassOf<UGameplayAbilityBase> AbilityToUse;
	FGridData Pattern;
	int DistToTarget = abs(PositionCoord.X - PlayerTarget->PositionCoord.X) + abs(PositionCoord.Y - PlayerTarget->PositionCoord.Y);

	for (UGameplayAbilityBase* Ability: GetAllAbilityInstances())
	{
		if (DistToTarget > Ability->Range) continue;
		AbilityToUse = Ability->GetClass();
		Pattern = Ability->Pattern;
		break;
	}

	if (!AbilityToUse) return false;
	CombatManager->EnemySetAttackInfo(AbilityToUse, AbilityDiceMap[AbilityToUse], Pattern, PlayerTarget->PositionCoord, EPatternRotation::R0);
	CombatManager->ExecuteAttackOnTarget();

	return true;
}

bool AEnemyEntity::IsTargetInAttackRange(int Range)
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	TArray<FIntVector2> AttackableTiles = GridManager->GetCellsInAttackRange(PositionCoord, Range, GetPathingData());
	for (int i = 0; i < AttackableTiles.Num(); i++)
	{
		if (!GridManager->GridCells[AttackableTiles[i]]->IsOccupied) continue;
		if (GridManager->GridCells[AttackableTiles[i]]->OccupyingActor == PlayerTarget) return true;
	}
	
	return false;
}
