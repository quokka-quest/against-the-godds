// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityBase.h"
#include "Kismet/GameplayStatics.h"
#include "CombatManager.h"
#include "AttributeHealthSet.h"

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
