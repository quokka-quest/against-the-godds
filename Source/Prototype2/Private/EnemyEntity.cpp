// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyEntity.h"
#include "CombatManager.h"
#include "PathFinder.h"
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

		FIntVector PlayerPos = Player->PositionCoord;
		int XDist = abs(PlayerPos.X - PositionCoord.X);
		int YDist = abs(PlayerPos.Y - PositionCoord.Y);
		int ZDist = abs(PlayerPos.Z - PositionCoord.Z);
		int totalDist = XDist + YDist + ZDist;

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

	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	TArray<FIntVector> PathToTarget = PathFinder(GridManager->GridCells).FindPathToPointInRangeOFTarget(PositionCoord, PlayerTarget->PositionCoord, MaxRange);

	// establish the first tile in the path
	// pathing gives the path in reverse to the last index is the start tile
	// the -2 gives the first tile to move to
	int TargetPosIndex = PathToTarget.Num() - 2;
	if (TargetPosIndex < 0) return;

	// if the target tile is occupied then movement can't be done
	if (GridManager->GridCells[PathToTarget[TargetPosIndex]]->IsOccupied) return;
	// If the movement cost is greater than the available movement of this enemy entity, then movement can't be done
	int MoveCost = GridManager->GridCells[PathToTarget[TargetPosIndex]]->MovementCost;
	if (MoveCost > MaxMovement) return;

	// -3 is done here because the .Num()-2 tile is done above to establish if movement is possible
	for (int i = PathToTarget.Num()-3; i >= 0; i--)
	{
		// breaks the loop if the tile has an entity on it (prevents movement onto an occupied tile)
		if (GridManager->GridCells[PathToTarget[i]]->IsOccupied) break;

		// check if the available movement will allow for this tile to be moved to
		int AddMoveCost = GridManager->GridCells[PathToTarget[i]]->MovementCost;
		if (MoveCost + AddMoveCost > MaxMovement) break;

		// if it can then the current tile is set as the new target
		MoveCost += AddMoveCost;
		TargetPosIndex = i;
	}

	AvailableMovement -= MoveCost;
	for (int i = PathToTarget.Num()-1; i > TargetPosIndex; i--)
	{
		FVector StartPos = GridManager->GridCells[PathToTarget[i]]->GetActorLocation();
		FVector EndPos = GridManager->GridCells[PathToTarget[i-1]]->GetActorLocation();
		EnqueueMovement(StartPos, EndPos);
	}

	// removes this entity from the cell it started in
	GridManager->GridCells[PositionCoord]->IsOccupied = false;
	GridManager->GridCells[PositionCoord]->OccupyingEntity = nullptr;

	// tells the cell it moved to that it is occupied
	PositionCoord = PathToTarget[TargetPosIndex];
	GridManager->GridCells[PositionCoord]->IsOccupied = true;
	GridManager->GridCells[PositionCoord]->OccupyingEntity = Cast<AEntityBase>(this);
}

bool AEnemyEntity::DetermineAttack()
{
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
	
	TSubclassOf<UGameplayAbilityBase> AbilityToUse;
	EAttackPattern Pattern = EAttackPattern::SingleTarget;
	int DistToTarget = abs(PositionCoord.X - PlayerTarget->PositionCoord.X) + abs(PositionCoord.Y - PlayerTarget->PositionCoord.Y);

	for (UGameplayAbilityBase* Ability: GetAllAbilityInstances())
	{
		if (DistToTarget > Ability->Range) continue;
		AbilityToUse = Ability->GetClass();
		Pattern = Ability->Pattern;
		break;
	}

	if (!AbilityToUse) return false;
	CombatManager->EnemySetAttackInfo(AbilityToUse, AbilityDiceMap[AbilityToUse], Pattern, PlayerTarget->PositionCoord, EAttackRotation::R0);
	CombatManager->ExecuteAttackOnTarget();

	return true;
}

bool AEnemyEntity::IsTargetInAttackRange(int Range)
{
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	TArray<FIntVector> AttackableTiles = PathFinder(GridManager->GridCells).FindAttackableTiles(PositionCoord, Range);
	for (int i = 0; i < AttackableTiles.Num(); i++)
	{
		if (!GridManager->GridCells[AttackableTiles[i]]->IsOccupied) continue;
		if (GridManager->GridCells[AttackableTiles[i]]->OccupyingEntity == PlayerTarget) return true;
	}
	
	return false;
}
