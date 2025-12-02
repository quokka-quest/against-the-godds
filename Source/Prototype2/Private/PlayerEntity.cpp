// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerEntity.h"
#include "GameManager.h"
#include "PersistentDataStruct.h"


void APlayerEntity::SendStatsToGameInstance(TSubclassOf<APlayerEntity> CharClass)
{
	UGameManager* GameInst = Cast<UGameManager>(GetGameInstance());
	if (!GameInst) {UE_LOG(LogTemp, Warning, TEXT("EntityBase->SendStatsToGameInstance: Failed to cast game instance")) return;}

	FPersistentPlayerInfo Info;
	Info.Abilities = Abilities;
	Info.AbilityDiceMap = AbilityDiceMap;
	Info.ActiveEffects = DefaultEffects;
	Info.MaxAttacks = MaxAttacks;
	Info.MaxMovement = MaxMovement;
	Info.MaxHealth = GetMaxHealth();
	Info.CurrentHealth = GetCurrentHealth();
	
	if (!GameInst->CharacterInfo.Contains(CharClass))
	{
		GameInst->CharacterInfo.Add(CharClass, Info);
		return;
	}

	GameInst->CharacterInfo[CharClass] = Info;
}