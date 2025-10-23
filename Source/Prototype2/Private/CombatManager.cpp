// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"

#include "Kismet/GameplayStatics.h"

void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	
	EnablePlayerLocationPicking();
}

void ACombatManager::EnablePlayerLocationPicking()
{
	GridManager->ToggleTileVisibility(EEditorGridDisplayType::PlayerSpawnTile);
}
