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

	// the TargetPosIndex is the index of the next cell to move onto in the pathing array.
	// since pathing gives a path from this enemy entity to the cell the target player is on the target position index is the second to last element in the path array
	// pathing gives the path in reverse so the last index is the start tile
	// the -2 gives the first tile to move to
	int TargetPosIndex = PathToTarget.Num() - 2;
	if (TargetPosIndex < 0) return;

	// pathing will give a path from the enemy to the player (ignoring occupancy) so when they are adjacent it will still return a valid path
	// this if statement prevents the enemy from walking onto the player's cell when they are directly next to each other
	if (GridManager->GridCells[PathToTarget[TargetPosIndex].CellCoordinate]->IsOccupied) return;
	// If the movement cost is greater than the available movement of this enemy entity, then movement can't be done
	int MoveCost = GridManager->GridCells[PathToTarget[TargetPosIndex].CellCoordinate]->MovementCost;
	if (MoveCost > MaxMovement) return;

	// this for loop establishes how far along the given path this entity can move. The for loop below does the actual movement
	// this is where any and all movement logic for enemy entities should be
	// -3 is done here because the .Num()-2 tile is done above to establish if movement is possible
	for (int i = PathToTarget.Num()-3; i >= 0; i--)
	{
		// breaks the loop if the tile has an entity on it (prevents movement onto an occupied tile)
		if (GridManager->GridCells[PathToTarget[i].CellCoordinate]->IsOccupied) break;

		// check if the available movement will allow for this tile to be moved to
		int AddMoveCost = GridManager->GridCells[PathToTarget[i].CellCoordinate]->MovementCost;
		if (MoveCost + AddMoveCost > MaxMovement) break;

		// if it can then the current tile is set as the new target
		MoveCost += AddMoveCost;
		TargetPosIndex = i; // the target position index changes to be the next cell to move onto
	}

	// this for loop calls for the actual movement
	AvailableMovement -= MoveCost;
	for (int i = PathToTarget.Num()-1; i > TargetPosIndex; i--)
	{
		bool rot = PathToTarget[i-1].FacingDirOnCell != PathToTarget[i-1].PrevFacingDir;
		FRotator StartRot = FacingDirectionRotations[PathToTarget[i-1].PrevFacingDir];
		FRotator EndRot = FacingDirectionRotations[PathToTarget[i-1].FacingDirOnCell];
		if (rot) EnqueueRotation(StartRot, EndRot);

		FVector StartPos = GridManager->GridCells[PathToTarget[i].CellCoordinate]->GetActorLocation();
		FVector EndPos = GridManager->GridCells[PathToTarget[i-1].CellCoordinate]->GetActorLocation();
		EnqueueMovement(StartPos, EndPos);
	}

	// removes this entity from the cell it started in
	ChangeOccupancy(PositionCoord, false);
	FacingDirection = PathToTarget[TargetPosIndex].FacingDirOnCell;

	// set occupancy on the cell that was moved onto
	ChangeOccupancy(PathToTarget[TargetPosIndex].CellCoordinate, true);
	PositionCoord = PathToTarget[TargetPosIndex].CellCoordinate;
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

void AEnemyEntity::ChangeOccupancy(FIntVector2 Coord, bool SetAsOccupier)
{
	AGridManagerTool* GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));

	for (FIntVector2 Offset : EntityRotations[FacingDirection].GetSelectedCellOffsets())
	{
		FIntVector2 CellCoord = Coord + Offset;
		GridManager->GridCells[CellCoord]->SetOccupier((SetAsOccupier)? this : nullptr);
	}
}
