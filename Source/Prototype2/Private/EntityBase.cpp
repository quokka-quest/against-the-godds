// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityBase.h"
#include "Kismet/GameplayStatics.h"
#include "CombatManager.h"
#include "AttributeHealthSet.h"

AEntityBase::AEntityBase()
{
	BaseplateMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseplateMesh");
	RootComponent = BaseplateMesh;

	CharacterMesh = CreateDefaultSubobject<UStaticMeshComponent>("CharacterMesh");
	CharacterMesh->SetupAttachment(BaseplateMesh);

	if (BaseMesh) BaseplateMesh->SetStaticMesh(BaseMesh);
	if (CharMesh) CharacterMesh->SetStaticMesh(CharMesh);

	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseplateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool AEntityBase::HasEntityDied()
{
	return (HealthSet->GetCurrentHealth() <= 0);
}

void AEntityBase::OnEntityDeath_Implementation()
{
	Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()))->OnEntityDeath(this);
}

void AEntityBase::PrintDebugData()
{
	UE_LOG(LogTemp, Warning, TEXT("Current Health: %f"), HealthSet->GetCurrentHealth());
}
