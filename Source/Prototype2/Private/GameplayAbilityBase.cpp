// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilityBase.h"
#include "CharacterBase.h"

float UGameplayAbilityBase::CalculateDamageWithMods(const float BaseDamage)
{
	const ACharacterBase* Owner = Cast<ACharacterBase>(GetOwningActorFromActorInfo());

	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbilityBase::CalculateDamageWithMods: Owner is null"));
		return BaseDamage;
	}

	// Creating these vars for damage calculation clarity
	// if this becomes a bottleneck lets just do the calc in one shot with no vars

	const float FlatDamageMod = Owner->GetFlatDamageModifier();
	const float MultiDamageMod = Owner->GetMultiDamageModifier();

	// Currently doing (Base + Flat) * Multi, we can change this for balacning reasons eg. (Base * Multi) + Flat
	const float NewDamage = (BaseDamage + FlatDamageMod) * MultiDamageMod;

	return NewDamage;
}

void UGameplayAbilityBase::RollDice()
{
}

TArray<AActor*> UGameplayAbilityBase::GetTargets() const
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
		// elif (NewTarget->IsA(ATile::StaticClass()))
		// {
		// 	if (TargetType != ETargetType::TT_Tile)
		// 	{
		// 		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbilityBase::GetTargets: TargetType mismatch, expected Character but got Tile"));
		// 		return TArray<AActor*>();
		// 	}
		// }
		else
		{
			return TArray<AActor*>();
		}
	}

	return Owner->GetTargets();
}
