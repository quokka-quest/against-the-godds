// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilityTargeted.h"
#include "GlobalDataTypeHeader.h"
#include "GridCellBase.h"
#include "CharacterBase.h"

TArray<AActor*> UGameplayAbilityTargeted::GetTargets() const
{
	const ACharacterBase* Owner = Cast<ACharacterBase>(GetOwningActorFromActorInfo());

	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbilityBase::GetTargets: Owner is null"));
		return TArray<AActor*>();
	}

	TArray<AActor*> NewTargets = Owner->GetTargets();
	
	for (AActor* NewTarget : NewTargets)
	{
		if (!NewTarget)
		{
			UE_LOG(LogTemp, Warning, TEXT("UGameplayAbilityBase::GetTargets: One of the targets is null"));
			return TArray<AActor*>();
		}

		if (NewTarget->IsA(ACharacterBase::StaticClass()))
		{
			if (TargetType != ETargetType::TT_Character)
			{
				UE_LOG(LogTemp, Warning, TEXT("UGameplayAbilityBase::GetTargets: TargetType mismatch, expected Tile but got Character"));
				return TArray<AActor*>();
			}
		}
		else if (NewTarget->IsA(AGridCellBase::StaticClass()))
		{
			if (TargetType != ETargetType::TT_Tile)
		 	{
		 		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbilityBase::GetTargets: TargetType mismatch, expected Character but got Tile"));
		 		return TArray<AActor*>();
		 	}
		}
		else
		{
			return TArray<AActor*>();
		}
	}

	return Owner->GetTargets();
}
