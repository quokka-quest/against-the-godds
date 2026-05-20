// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityBase.h"
#include "Kismet/GameplayStatics.h"
#include "CombatManager.h"
#include "GameManager.h"
#include "AttributeHealthSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

AEntityBase::AEntityBase()
{
	DirectionYaws.Add(R0, 90.0f);
	DirectionYaws.Add(R90, 0.0f);
	DirectionYaws.Add(R180, 270.0f);
	DirectionYaws.Add(R270, 180.0f);
}

bool AEntityBase::HasEntityDied()
{
	return (HealthSet->GetCurrentHealth() <= 0);
}

void AEntityBase::OnEntityDeath_Implementation()
{
	Cast<ACombatManager>(UGameplayStatics::GetGameMode(GetWorld()))->OnEntityDeath(this);
}

void AEntityBase::SetCharacterData(FPersistentPlayerInfo Info)
{
	Abilities = Info.Abilities;
	AbilityDiceMap = Info.AbilityDiceMap;
	DefaultEffects = Info.ActiveEffects;

	SetMaxAttacks(Info.MaxAttacks);
	SetMaxMovement(Info.MaxMovement);

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

void AEntityBase::AddDraftedAbilityToCharacter(UGameplayAbilityBase* DraftedAbility)
{
	if (!DraftedAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddDraftedAbilityToCharacter: DraftedAbility was null"));
		return;
	}

	// The persistent Abilities array stores classes, not instances
	TSubclassOf<UGameplayAbility> AbilityClass = DraftedAbility->GetClass();

	// Avoid duplicates in the stored ability array
	if (!Abilities.Contains(AbilityClass))
	{
		Abilities.Add(AbilityClass);
	}

	// Grant the ability to the live Ability System Component
	if (AbilitySystemComponent)
	{
		bool bAlreadyGranted = false;

		for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilitySpec.Ability && AbilitySpec.Ability->GetClass() == AbilityClass)
			{
				bAlreadyGranted = true;
				break;
			}
		}

		if (!bAlreadyGranted)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this)
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AddDraftedAbilityToCharacter: AbilitySystemComponent was null"));
	}

	// AbilityDiceMap uses GameplayAbilityBase classes as keys
	TSubclassOf<UGameplayAbilityBase> AbilityBaseClass = DraftedAbility->GetClass();

	if (!AbilityDiceMap.Contains(AbilityBaseClass))
	{
		FDiceFaceLevels StartingDiceLevels;
		StartingDiceLevels.FaceArray.Reserve(6);

		for (int32 FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
		{
			FDiceFaceValues FaceValues;
			FaceValues.PrimaryLevel = 1;
			FaceValues.SecondaryLevel = 0;
			FaceValues.TertiaryLevel = 0;
			FaceValues.FourthLevel = 0;
			FaceValues.FifthLevel = 0;

			StartingDiceLevels.FaceArray.Add(FaceValues);
		}

		AbilityDiceMap.Add(AbilityBaseClass, StartingDiceLevels);
	}
}

void AEntityBase::InitialiseStats()
{
	HealthSet->SetCurrentHealth(HealthSet->GetMaxHealth());
}


void AEntityBase::PrintDebugData()
{
	UE_LOG(LogTemp, Warning, TEXT("Current Health: %f"), HealthSet->GetCurrentHealth());
}

bool AEntityBase::ReplaceAbilityOnCharacter(
	UGameplayAbilityBase* AbilityToReplace,
	UGameplayAbilityBase* NewDraftedAbility
)
{
	if (!AbilityToReplace)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: AbilityToReplace was null"));
		return false;
	}

	if (!NewDraftedAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: NewDraftedAbility was null"));
		return false;
	}

	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: AbilitySystemComponent was null"));
		return false;
	}

	TSubclassOf<UGameplayAbility> OldAbilityClass = AbilityToReplace->GetClass();
	TSubclassOf<UGameplayAbility> NewAbilityClass = NewDraftedAbility->GetClass();

	const int32 ExistingAbilityIndex = Abilities.Find(OldAbilityClass);

	if (ExistingAbilityIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: Old ability class was not found in Abilities"));
		return false;
	}

	if (Abilities.Contains(NewAbilityClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: New ability already exists in Abilities"));
		return false;
	}

	// Find the live granted GAS ability spec that matches the old ability
	FGameplayAbilitySpecHandle OldAbilityHandle;
	bool bFoundOldGrantedAbility = false;

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.Ability && AbilitySpec.Ability->GetClass() == OldAbilityClass)
		{
			OldAbilityHandle = AbilitySpec.Handle;
			bFoundOldGrantedAbility = true;
			break;
		}
	}

	if (!bFoundOldGrantedAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: Old ability was not found on AbilitySystemComponent"));
		return false;
	}

	// Clear old live ability
	AbilitySystemComponent->ClearAbility(OldAbilityHandle);

	// Grant new live ability
	const FGameplayAbilitySpecHandle NewAbilityHandle =
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(NewAbilityClass, 1, INDEX_NONE, this)
		);

	if (!NewAbilityHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ReplaceAbilityOnCharacter: Failed to grant new ability to AbilitySystemComponent"));
		return false;
	}

	// Only update persistent/stored data once the live ASC change succeeded
	Abilities[ExistingAbilityIndex] = NewAbilityClass;

	TSubclassOf<UGameplayAbilityBase> OldAbilityBaseClass = AbilityToReplace->GetClass();
	TSubclassOf<UGameplayAbilityBase> NewAbilityBaseClass = NewDraftedAbility->GetClass();

	AbilityDiceMap.Remove(OldAbilityBaseClass);

	FDiceFaceLevels StartingDiceLevels;
	StartingDiceLevels.FaceArray.Reserve(6);

	for (int32 FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
	{
		FDiceFaceValues FaceValues;
		FaceValues.PrimaryLevel = 1;
		FaceValues.SecondaryLevel = 0;
		FaceValues.TertiaryLevel = 0;
		FaceValues.FourthLevel = 0;
		FaceValues.FifthLevel = 0;

		StartingDiceLevels.FaceArray.Add(FaceValues);
	}

	AbilityDiceMap.Add(NewAbilityBaseClass, StartingDiceLevels);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("ReplaceAbilityOnCharacter: Successfully replaced %s with %s"),
		*GetNameSafe(OldAbilityClass),
		*GetNameSafe(NewAbilityClass)
	);

	return true;
}
