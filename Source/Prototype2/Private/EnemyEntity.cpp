// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyEntity.h"
#include "CombatManager.h"
#include "Kismet/GameplayStatics.h"

AEnemyEntity::AEnemyEntity()
{
	CharMesh = GetCharMesh();
	CharacterMesh->SetStaticMesh(CharMesh);
}


void AEnemyEntity::DeterminePlayerTarget()
{
	ACombatManager* CombatManager = Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()));
	
	int ShortestDist = 100000;
	APlayerEntity* ClosestPlayer = nullptr;
	
	for (AEntityBase* Entity : CombatManager->Combatants)
	{
		APlayerEntity* Player = Cast<APlayerEntity>(Entity);
		if (!Player) continue;

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
	// if the player target is invalid something has gone wrong so do nothing
	if (!PlayerTarget) return;

	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	TArray<FIntVector> PathToTarget = GridManager->GetPath(PositionCoord, PlayerTarget->PositionCoord);

	// establish the first tile in the path
	int TargetPosIndex = PathToTarget.Num() - 2;
	// If the player and enemy are on adjacent tiles then this array will contain 2 elements, neither of which the enemy can move to
	if (TargetPosIndex <= 0) return;

	// If the movement cost is greater than the available movement of this enemy entity, then movement can't be done
	int MoveCost = GridManager->GridCells[PathToTarget[TargetPosIndex]]->MovementCost;
	if (MoveCost > MaxMovement) return;

	
	for (int i = PathToTarget.Num()-3; i > 0; i--)
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
	PositionCoord = PathToTarget[TargetPosIndex];
}


