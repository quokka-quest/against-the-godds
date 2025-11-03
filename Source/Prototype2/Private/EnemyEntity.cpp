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
	if (!PlayerTarget) return;

	int XDiff = abs(PlayerTarget->PositionCoord.X - PositionCoord.X);
	int YDiff = abs(PlayerTarget->PositionCoord.Y - PositionCoord.Y);
	int ZDiff = abs(PlayerTarget->PositionCoord.Z - PositionCoord.Z);

	int XYDiff = XDiff + YDiff;
	if (XYDiff == 1 && ZDiff <= 1) return;

	
}


