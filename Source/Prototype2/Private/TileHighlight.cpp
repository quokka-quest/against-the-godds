// Fill out your copyright notice in the Description page of Project Settings.


#include "TileHighlight.h"

// Sets default values
ATileHighlight::ATileHighlight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	HighlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightMesh"));
	HighlightMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Levels/_GENERATED/kibbl/SM_TileHighlight.SM_TileHighlight"));
	if (!MeshAsset.Succeeded()) {UE_LOG(LogTemp, Error, TEXT("Failed to load mesh asset for highlight")) return;}

	HighlightMesh->SetStaticMesh(MeshAsset.Object);
}

// Called when the game starts or when spawned
void ATileHighlight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATileHighlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

