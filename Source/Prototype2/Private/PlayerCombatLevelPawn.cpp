// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatLevelPawn.h"

#include "GridCell.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerCombatLevelPawn::APlayerCombatLevelPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HighlightedCell = nullptr;
	PlayerCon = nullptr;
	TileHighlight = nullptr;

	TileSelectionType = ETileSelectionType::SpawnSelection;

}

// Called when the game starts or when spawned
void APlayerCombatLevelPawn::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (!GridManager) {UE_LOG(LogTemp, Error, TEXT("GridManager is NULL")) return;}

	TileHighlight = Cast<ATileHighlight>(UGameplayStatics::GetActorOfClass(GetWorld(), ATileHighlight::StaticClass()));
	if (!TileHighlight) {UE_LOG(LogTemp, Error, TEXT("TileHighlight is null")) return;}
	TileHighlight->SetActorLocation(FVector(0, 0, 0));
	
	PlayerCon = Cast<APlayerController>(GetController());
	if (!PlayerCon) {UE_LOG(LogTemp, Error, TEXT("PlayerController is null")) return;}

	PlayerCon->bShowMouseCursor = true;
}

// Called every frame
void APlayerCombatLevelPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult Hit;
	PlayerCon->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit);

	if (!Cast<AGridCell>(Hit.GetActor()))
	{
		TileHighlight->ToggleHighlight(false);
		HighlightedCell = nullptr;
		return;
	}
	
	HighlightedCell = Cast<AGridCell>(Hit.GetActor());
	TileHighlight->ToggleHighlight(true);
	TileHighlight->MoveToPosition(HighlightedCell->GetActorLocation());
}

// Called to bind functionality to input
void APlayerCombatLevelPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCombatLevelPawn::OnTileClick()
{
	if (!HighlightedCell) return;

	if (TileSelectionType == ETileSelectionType::SpawnSelection) TryAddTileToSpawnSelection();
	
}

void APlayerCombatLevelPawn::TryAddTileToSpawnSelection()
{
	int playerCount = 3;

	// if the tile clicked is not a valid spawn tile then do nothing
	if (!HighlightedCell->IsPlayerSpawnTile) return;

	// if the tile clicked is already in the spawn array then remove it
	if (SelectedStartCells.Contains(HighlightedCell))
	{
		SelectedStartCells.Remove(HighlightedCell);
		GridManager->ChangeTilesMaterial(HighlightedCell, ETileMaterial::Highlighted);
		return;
	};

	// if the spawn tile array is full then do nothing
	if (SelectedStartCells.Num() >= playerCount) return;

	// otherwise add the tile to the spawn tile array
	SelectedStartCells.Add(HighlightedCell);
	GridManager->ChangeTilesMaterial(HighlightedCell, ETileMaterial::Target);
}


