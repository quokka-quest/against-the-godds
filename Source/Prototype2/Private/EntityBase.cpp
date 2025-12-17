// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityBase.h"
#include "Kismet/GameplayStatics.h"
#include "CombatManager.h"
#include "GameManager.h"
#include "AttributeHealthSet.h"

AEntityBase::AEntityBase()
{
	FacingDirectionRotations.Add(R0, FRotator(0, 90, 0));
	FacingDirectionRotations.Add(R90, FRotator(0, 0, 0));
	FacingDirectionRotations.Add(R180, FRotator(0, -90, 0));
	FacingDirectionRotations.Add(R270, FRotator(0, 180, 0));
}


bool AEntityBase::HasEntityDied()
{
	return (HealthSet->GetCurrentHealth() <= 0);
}

void AEntityBase::OnEntityDeath_Implementation()
{
	Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()))->OnEntityDeath(this);
}

void AEntityBase::SetCharacterData(FPersistentPlayerInfo& Info)
{
	Abilities = Info.Abilities;
	AbilityDiceMap = Info.AbilityDiceMap;
	DefaultEffects = Info.ActiveEffects;

	MaxAttacks = Info.MaxAttacks;
	MaxMovement = Info.MaxMovement;

	HealthSet->SetMaxHealth(Info.MaxHealth);
	HealthSet->SetCurrentHealth(Info.CurrentHealth);
}

FPathingData AEntityBase::GetPathingData()
{
	FPathingData Result;
	Result.Actor = this;
	Result.ActorRotations = EntityRotations;
	Result.RotationSweep = RotationSweep;
	Result.CurrentRotation = FacingDirection;
	return Result;
}

void AEntityBase::PrintDebugData()
{
	UE_LOG(LogTemp, Warning, TEXT("Current Health: %f"), HealthSet->GetCurrentHealth());
}
