// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatManager.h"
#include "PlayerEntity.h"
#include "Kismet/GameplayStatics.h"

// Sets references to needed manager classes on start
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	GridManager->ChangeAllTilesDisplay(EEditorGridDisplayType::PlayerSpawnTile);
}

// Called when the player locks in there start location choices
// spawns the player characters on the chosen tiles
void ACombatManager::FinishPlayerLocationPicking(TArray<AGridCell*> &playerStartCells)
{
	for (AGridCell*& Cell : playerStartCells)
	{
		FVector Loc = Cell->GetActorLocation();
		FRotator Rot = Cell->GetActorRotation();
		FTransform Transform = FTransform(Rot, Loc);
		GetWorld()->SpawnActor<APlayerEntity>(APlayerEntity::StaticClass(), Transform);
	}
}

