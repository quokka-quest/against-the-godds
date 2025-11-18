// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatLevelPawn.h"
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
	SelectedCell = nullptr;
	CombatManager = nullptr;
	GridManager = nullptr;

	TileSelectionType = ETileSelectionType::SpawnSelection;
	IsDisplayingAttack = false;
}

// Called when the game starts or when spawned
void APlayerCombatLevelPawn::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));
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
	CombatManager->OnAttackExecuted.AddDynamic(this, &APlayerCombatLevelPawn::OnAttackExecuted);
}

// Called every frame
void APlayerCombatLevelPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InFreeCamMode) return;
	
	FHitResult Hit;
	PlayerCon->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit);

	if (!Cast<AGridCellParent>(Hit.GetActor()))
	{
		TileHighlight->ToggleHighlight(false);
		HighlightedCell = nullptr;
		return;
	}
	
	HighlightedCell = Cast<AGridCellParent>(Hit.GetActor());
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


void APlayerCombatLevelPawn::OnTileClick()
{
	if (!HighlightedCell) {OnClickedOffTileGrid(); return;}
	if (TileSelectionType == None) return;
	
	if (TileSelectionType == ETileSelectionType::SpawnSelection) TryAddTileToSpawnSelection();

	// click logic for when the selected cell is not the cell being clicked on
	if (SelectedCell != HighlightedCell)
	{
		SelectedCell = HighlightedCell;
		
		if (TileSelectionType == ETileSelectionType::Movement) DisplayPathToTile();
		if (TileSelectionType == ETileSelectionType::Attack) DisplayAttackTargetArea();
		
		return;
	}

	// click logic for when the selected cell is clicked
	if (TileSelectionType == ETileSelectionType::Movement) TryMoveToTile();
	if (TileSelectionType == ETileSelectionType::Attack) TryAttackTargetTile();
	
}

void APlayerCombatLevelPawn::TryAddTileToSpawnSelection()
{
	int playerCount = 3;

	
	// if the tile clicked is not a valid spawn tile then do nothing
	if (!HighlightedCell) return;
	if (!HighlightedCell->IsPlayerSpawnCell) return;

	// if the tile clicked is already in the spawn array then remove it
	if (SelectedStartCells.Contains(HighlightedCell))
	{
		SelectedStartCells.Remove(HighlightedCell);
		GridManager->ChangeCellsMaterial(HighlightedCell, ETileMaterial::Highlighted);
		return;
	};

	// if the spawn tile array is full then do nothing
	if (SelectedStartCells.Num() >= playerCount) return;

	// otherwise add the tile to the spawn tile array
	SelectedStartCells.Add(HighlightedCell);
	GridManager->ChangeCellsMaterial(HighlightedCell, ETileMaterial::Target);
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
	if (!HighlightedCell->IsWalkable) return;

	CombatManager->MoveCurrentCombatant(HighlightedCell->CellCoordinate);
	TileSelectionType = ETileSelectionType::None;
	SelectedCell = nullptr;
}

void APlayerCombatLevelPawn::DisplayPathToTile()
{
	// this resets the tiles (clears the movement path if it's displayed)
	CombatManager->DisplayCurrentCombatantsMovement();

	// no movement path is displayed if the highlighted cell is none-walkable
	if (!HighlightedCell->IsWalkable) return;

	// displays the path to the cell that was clicked
	CombatManager->DisplayPathForCurrentCombatant(HighlightedCell->CellCoordinate);
}

void APlayerCombatLevelPawn::DisplayAttackTargetArea()
{
	if (!HighlightedCell) return;
	if (!HighlightedCell->IsAttackable) return;

	SelectedCell = HighlightedCell;
	IsDisplayingAttack = true;
	CombatManager->DisplayAttackPattern(HighlightedCell->CellCoordinate);
}

void APlayerCombatLevelPawn::TryAttackTargetTile()
{
	CombatManager->ExecuteAttackOnTarget();
}


void APlayerCombatLevelPawn::OnPlayerTurnEnd()
{
	SelectedCell = nullptr;
	TileSelectionType = ETileSelectionType::None;
}

void APlayerCombatLevelPawn::OnMoveButtonClicked()
{
	SelectedCell = nullptr;
	IsDisplayingAttack = false;
	TileSelectionType = ETileSelectionType::Movement;
}

void APlayerCombatLevelPawn::OnAttackButtonClicked()
{
	SelectedCell = nullptr;
	IsDisplayingAttack = false;
	TileSelectionType = ETileSelectionType::Attack;
}

void APlayerCombatLevelPawn::OnRotateAttack()
{
	EPatternRotation Rot = CombatManager->GetAttackRotation();
	EPatternRotation NewRot = (Rot == R0)? R90: (Rot == R90)? R180: (Rot == R180)? R270 : R0;
	CombatManager->SetAttackRotation(NewRot);

	if (IsDisplayingAttack)
	{
		DisplayAttackTargetArea();
	}
}

// This function holds the logic for when the player clicks off of the tile grid
// Used for resetting the tile display for pathfinding and attack targeting
void APlayerCombatLevelPawn::OnClickedOffTileGrid()
{
	SelectedCell = nullptr;

	if (TileSelectionType == ETileSelectionType::None) return;
	if (TileSelectionType == ETileSelectionType::SpawnSelection) return;
	if (TileSelectionType == ETileSelectionType::Movement) {CombatManager->DisplayCurrentCombatantsMovement(); return;}
	if (TileSelectionType == ETileSelectionType::Attack) return; // can implement target pattern reset when merged with Lee-Roy's GAS system
}

void APlayerCombatLevelPawn::OnAttackExecuted()
{
	TileSelectionType = ETileSelectionType::None;
}




