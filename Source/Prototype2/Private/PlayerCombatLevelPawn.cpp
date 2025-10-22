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

}

// Called when the game starts or when spawned
void APlayerCombatLevelPawn::BeginPlay()
{
	Super::BeginPlay();

	TileHighlight = Cast<ATileHighlight>(UGameplayStatics::GetActorOfClass(GetWorld(), ATileHighlight::StaticClass()));
	if (!TileHighlight) {UE_LOG(LogTemp, Error, TEXT("TileHighlight is null")) return;}
	
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
	if (!Cast<AGridCell>(Hit.GetActor())) {return;}

	HighlightedCell = Cast<AGridCell>(Hit.GetActor());
	TileHighlight->SetActorLocation(HighlightedCell->GetActorLocation());
}

// Called to bind functionality to input
void APlayerCombatLevelPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

