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
	if (!GridManager) UE_LOG(LogTemp, Error, TEXT("GridManager is NULL"))

	CombatManager = Cast<ACombatManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
	if (!CombatManager) UE_LOG(LogTemp, Error, TEXT("CombatManager is NULL"))

	TileHighlight = Cast<ATileHighlight>(UGameplayStatics::GetActorOfClass(GetWorld(), ATileHighlight::StaticClass()));
	if (!TileHighlight) UE_LOG(LogTemp, Error, TEXT("TileHighlight is null"))
	TileHighlight->SetActorLocation(FVector(0, 0, 0));
	
	PlayerCon = Cast<APlayerController>(GetController());
	if (!PlayerCon) UE_LOG(LogTemp, Error, TEXT("PlayerController is null"))

	PlayerCon->bShowMouseCursor = true;

	CombatManager->OnPlayerTurnEnd.AddDynamic(this, &APlayerCombatLevelPawn::OnPlayerTurnEnd);
	CombatManager->OnMoveButtonClicked.AddDynamic(this, &APlayerCombatLevelPawn::OnMoveButtonClicked);
	CombatManager->OnAttackButtonClicked.AddDynamic(this, &APlayerCombatLevelPawn::OnAttackButtonClicked);
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
// Even though it does nothing, removing this breaks things
void APlayerCombatLevelPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCombatLevelPawn::SetTileSelectionType(ETileSelectionType Type)
{
	TileSelectionType = Type;
}

// need to add the 'attack' tile selection type functionality to this
void APlayerCombatLevelPawn::OnTileClick()
{
	if (!HighlightedCell) {SelectedCell = nullptr; return;}

	if (TileSelectionType == ETileSelectionType::SpawnSelection) TryAddTileToSpawnSelection();
	
	if (SelectedCell != HighlightedCell)
	{
		SelectedCell = HighlightedCell;
		
		if (TileSelectionType == ETileSelectionType::Movement) DisplayPathToTile();
		if (TileSelectionType == ETileSelectionType::Attack) DisplayAttackTargetArea();
		
		return;
	}

	if (TileSelectionType == ETileSelectionType::Movement) TryMoveToTile();
	if (TileSelectionType == ETileSelectionType::Attack) TryAttackTargetTile();
	
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

bool APlayerCombatLevelPawn::AttemptToFinishPlayerStartPlacement()
{
	int playerCount = 3;
	if (SelectedStartCells.Num() != playerCount) return false;

	CombatManager->FinishPlayerLocationPicking(SelectedStartCells);
	return true;
}

void APlayerCombatLevelPawn::TryMoveToTile()
{
	if (!HighlightedCell) return;
	if (!HighlightedCell->isWalkable) return;

	CombatManager->MoveCurrentCombatant(HighlightedCell->GridCellCoord);
	TileSelectionType = ETileSelectionType::None;
	SelectedCell = nullptr;
}

void APlayerCombatLevelPawn::DisplayPathToTile()
{
	if (!HighlightedCell) return;
	if (!HighlightedCell->isWalkable) return;

	CombatManager->DisplayCurrentCombatantsMovement();
	CombatManager->DisplayPathForCurrentCombatant(HighlightedCell->GridCellCoord);
}

void APlayerCombatLevelPawn::DisplayAttackTargetArea()
{
	if (!HighlightedCell) return;
	if (!HighlightedCell->isAttackable) return;

	CombatManager->DisplayAttackPattern(HighlightedCell->GridCellCoord);
}

void APlayerCombatLevelPawn::TryAttackTargetTile()
{
	UE_LOG(LogTemp, Warning, TEXT("try to attack target tile"))
}


void APlayerCombatLevelPawn::OnPlayerTurnEnd()
{
	SelectedCell = nullptr;
	TileSelectionType = ETileSelectionType::None;
}

void APlayerCombatLevelPawn::OnMoveButtonClicked()
{
	SelectedCell = nullptr;
	TileSelectionType = ETileSelectionType::Movement;
}

void APlayerCombatLevelPawn::OnAttackButtonClicked()
{
	SelectedCell = nullptr;
	TileSelectionType = ETileSelectionType::Attack;
}





