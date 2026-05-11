// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCombatLevelPawn.h"
#include "Engine/Engine.h"
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

	// if the hovered cell is invalid or the mouse is hovering a cell that can not be highlighted (outside walkable or targetable area) or if the game is paused
	// then hide the highlight actor
	if (!HoveredCell ||
		(TileSelectionType == ETileSelectionType::Movement && !HoveredCell->IsWalkable) ||
		(isDisplayingPath && HoveredCell != SelectedCell) ||
		(TileSelectionType == ETileSelectionType::Attack && !HoveredCell->IsAttackable) ||
		(IsDisplayingAttack && HoveredCell != SelectedCell) ||
		UGameplayStatics::IsGamePaused(GetWorld()))
	{
		GridManager->SetHighlightVisibility(false);
		HighlightedCell = nullptr;
		return;
	}

	// If displaying a movement area, only highlight cells inside that area
	if (isDisplayingPath && !HoveredCell->IsWalkable) return;

	// if displaying a targetable area, only highlight cells inside that area
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
		if (TileSelectionType == ETileSelectionType::Movement)
		{
			if (isDisplayingPath) TurnOffPathDisplay();
			else if (GetCurrentCombatantGridPos() != HighlightedCell->CellCoordinate) { SelectedCell = HighlightedCell; DisplayPathToTile(); }
		}
		else if (TileSelectionType == ETileSelectionType::Attack) 
		{
			if (IsDisplayingAttack) TurnOffAttackDisplay();
			else { SelectedCell = HighlightedCell; DisplayAttackTargetArea(); }
		}
		
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
	ResetDisplayVariables(ETileSelectionType::None);
}

void APlayerCombatLevelPawn::OnMoveButtonClicked()
{
	ResetDisplayVariables(ETileSelectionType::Movement);
}

void APlayerCombatLevelPawn::OnAttackButtonClicked()
{
	ResetDisplayVariables(ETileSelectionType::Attack);
}

void APlayerCombatLevelPawn::ResetDisplayVariables(ETileSelectionType SelectionType)
{
	SelectedCell = nullptr;
	SelfTargetAbilityOnDisplay = nullptr;
	
	IsDisplayingAttack = false;
	isDisplayingPath = false;
	IsDisplayingSelfTargetArea = false;
	
	TileSelectionType = SelectionType;
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
	ResetDisplayVariables(TileSelectionType);

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
	CombatManager->DisplayAttackRange(CombatManager->AbilityRef->Range);
	TileSelectionType = ETileSelectionType::Attack;
	IsDisplayingAttack = false;
}

void APlayerCombatLevelPawn::TurnOffPathDisplay()
{
	CombatManager->DisplayCurrentCombatantsMovement();
	isDisplayingPath = false;
}

FVector APlayerCombatLevelPawn::GetMouseGroundIntersection()
{
	FVector MouseGroundIntersection = FVector::ZeroVector;

	// retrieve the mouse world position and direction
	FVector WorldPos;
	FVector WorldDir;
	PlayerCon->DeprojectMousePositionToWorld(WorldPos, WorldDir);

	// finds the intersection point with the plane z = 10
	float T = (10.0f - WorldPos.Z) / WorldDir.Z;
	MouseGroundIntersection = WorldPos + (T * WorldDir);
	
	return MouseGroundIntersection;
}

FIntVector2 APlayerCombatLevelPawn::GetCurrentCombatantGridPos() 
{
	return CombatManager->CurrentTurnCombatant->PositionCoord;
}

void APlayerCombatLevelPawn::SetSelfTargetInfo(AGridCellParent* Cell, UGameplayAbilityBase* Ability)
{
	if (!Cell) return;
	
	SelectedCell = Cell;
	SelfTargetAbilityOnDisplay = Ability;
	
	IsDisplayingAttack = true;
	IsDisplayingSelfTargetArea = true;
}

bool APlayerCombatLevelPawn::IsDisplayingSelfTargetAbility(UGameplayAbilityBase* Ability)
{
	return SelfTargetAbilityOnDisplay == Ability;
}

void APlayerCombatLevelPawn::TryUseSelfTargetAbility()
{
	TryAttackTargetTile();
	ResetDisplayVariables(ETileSelectionType::None);
}
