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
	SelectedCell = nullptr;
	CombatManager = nullptr;
	GridManager = nullptr;

	TileSelectionType = ETileSelectionType::None;
	IsDisplayingAttack = false;
	isDisplayingPath = false;
}

// Called when the game starts or when spawned
void APlayerCombatLevelPawn::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManagerTool>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManagerTool::StaticClass()));
	if (!GridManager) {UE_LOG(LogTemp, Error, TEXT("GridManager is NULL")) return;}

	CombatManager = Cast<ACombatManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
	if (!CombatManager) {UE_LOG(LogTemp, Error, TEXT("CombatManager is NULL")) return;}
	
	PlayerCon = Cast<APlayerController>(GetController());
	if (!PlayerCon) {UE_LOG(LogTemp, Error, TEXT("PlayerController is null")) return;}

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
	
	FHitResult Hit;
	PlayerCon->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit);

    AGridCellParent* HoveredCell = Cast<AGridCellParent>(Hit.GetActor());

	if (!HoveredCell ||
		(TileSelectionType == ETileSelectionType::Movement && !HoveredCell->IsWalkable) ||
		(isDisplayingPath && HoveredCell != SelectedCell) ||
		(TileSelectionType == ETileSelectionType::Attack && !HoveredCell->IsAttackable) ||
		(IsDisplayingAttack && HoveredCell != SelectedCell))
	{
		GridManager->SetHighlightVisibility(false);
		HighlightedCell = nullptr;
		return;
	}
	
	if (isDisplayingPath && !HoveredCell->IsWalkable) return;
	if (IsDisplayingAttack && !HoveredCell->IsAttackable) return;
	
	HighlightedCell = HoveredCell;
	GridManager->SetHighlightVisibility(true);
	GridManager->SetHighlightPosition(HighlightedCell->CellCoordinate);
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

	// click logic for when the selected cell is not the cell being clicked on
	if (SelectedCell != HighlightedCell)
	{
		if (!isDisplayingPath && !IsDisplayingAttack) SelectedCell = HighlightedCell;

		if (TileSelectionType == ETileSelectionType::Movement && isDisplayingPath) { TurnOffPathDisplay(); return; }
		if (TileSelectionType == ETileSelectionType::Attack && IsDisplayingAttack) { TurnOffAttackDisplay(); return; }
		
		if (TileSelectionType == ETileSelectionType::Movement) DisplayPathToTile();
		if (TileSelectionType == ETileSelectionType::Attack) DisplayAttackTargetArea();
		
		return;
	}

	// click logic for when the selected cell is clicked
	if (TileSelectionType == ETileSelectionType::Movement) TryMoveToTile();
	if (TileSelectionType == ETileSelectionType::Attack) TryAttackTargetTile();
	
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

	isDisplayingPath = true;
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
	IsDisplayingAttack = false;
	isDisplayingPath = false;
}

void APlayerCombatLevelPawn::OnMoveButtonClicked()
{
	SelectedCell = nullptr;
	IsDisplayingAttack = false;
	isDisplayingPath = false;
	TileSelectionType = ETileSelectionType::Movement;
}

void APlayerCombatLevelPawn::OnAttackButtonClicked()
{
	SelectedCell = nullptr;
	IsDisplayingAttack = false;
	isDisplayingPath = false;
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
	IsDisplayingAttack = false;
	isDisplayingPath = false;

	if (TileSelectionType == ETileSelectionType::None) return;
	if (TileSelectionType == ETileSelectionType::SpawnSelection) return;
	if (TileSelectionType == ETileSelectionType::Movement) { TurnOffPathDisplay(); return; }
	if (TileSelectionType == ETileSelectionType::Attack) { TurnOffAttackDisplay(); }
}

void APlayerCombatLevelPawn::OnAttackExecuted()
{
	TileSelectionType = ETileSelectionType::None;
	IsDisplayingAttack = false;
}

void APlayerCombatLevelPawn::TurnOffAttackDisplay()
{
	GridManager->ResetHighlights();
	FGridData DefaultHighlight = FGridData();
	GridManager->ChangeHighlightMesh(DefaultHighlight);
	TileSelectionType = ETileSelectionType::None;
	IsDisplayingAttack = false;
}

void APlayerCombatLevelPawn::TurnOffPathDisplay()
{
	CombatManager->DisplayCurrentCombatantsMovement();
	isDisplayingPath = false;
}


