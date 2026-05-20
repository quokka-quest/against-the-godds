// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AttributeHealthSet.h"
#include "AttributeDamageModifiersSet.h"
#include "AttributeTurnActionSet.h"
#include "GameplayAbilityBase.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the GAS components
	AbilitySystemComponent = CreateDefaultSubobject<UTurnBasedAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	HealthSet = CreateDefaultSubobject<UAttributeHealthSet>(TEXT("HealthSet"));
	DamageModifiersSet = CreateDefaultSubobject<UAttributeDamageModifiersSet>(TEXT("DamageModifiersSet"));
	TurnActionSet = CreateDefaultSubobject<UAttributeTurnActionSet>(TEXT("TurnActionSet"));
	StartFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.StartOfTurn")));
	EndFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.EndOfTurn")));
	StatusFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status")));
	BuffFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Buff")));
	DebuffFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Debuff")));
	RemoveAllStartOfTurnStacksTag = FGameplayTag::RequestGameplayTag(FName("Effect.StartOfTurn.RemoveAllStacks"));
	RemoveAllEndOfTurnStacksTag = FGameplayTag::RequestGameplayTag(FName("Effect.EndOfTurn.RemoveAllStacks"));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Subscribe the OnDamageTakenChanged method to the OnDamageTaken delegate (custom delegate) from the HealthSet
	HealthSet->OnDamageTaken.AddUObject(this, &ACharacterBase::OnDamageTakenChanged);
	// Also subscribe the OnCurrentHealthAttributeChanged to the health changing delegate from HealthSet
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthSet->GetCurrentHealthAttribute()).AddUObject(this, &ACharacterBase::OnCurrentHealthAttributeChanged);
}

TMap<FGameplayTag, int32> ACharacterBase::GetActiveStatusEffects() const
{
	TMap<FGameplayTag, int32> StatusEffects;
    
	if (!AbilitySystemComponent)
	{
		return StatusEffects;
	}
    
	// Get all active effects
	TArray<FActiveGameplayEffectHandle> ActiveEffects = AbilitySystemComponent->GetActiveEffectsWithAllTags(StatusFilterTags);
    
	for (const FActiveGameplayEffectHandle& EffectHandle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(EffectHandle);
		if (ActiveEffect && ActiveEffect->Spec.Def)
		{
			// Get the asset tags from the effect
			FGameplayTagContainer AssetTags;
			ActiveEffect->Spec.GetAllAssetTags(AssetTags);
            
			// Get stack count
			int32 StackCount = ActiveEffect->Spec.GetStackCount();
            
			// Store each tag with its stack count
			for (const FGameplayTag& Tag : AssetTags)
			{
				// Only add tags that match the "Status" category
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Status"))))
				{
					StatusEffects.Add(Tag, StackCount);
				}
			}
		}
	}
    
	return StatusEffects;
}

TMap<FGameplayTag, int32> ACharacterBase::GetActiveBuffs() const
{
	TMap<FGameplayTag, int32> Buffs;

	// early return if ability system component is invalid
	if (!AbilitySystemComponent) return Buffs;
    
	// Get all active effects
	TArray<FActiveGameplayEffectHandle> ActiveBuffs = AbilitySystemComponent->GetActiveEffectsWithAllTags(BuffFilterTags);
    
	for (const FActiveGameplayEffectHandle& EffectHandle : ActiveBuffs)
	{
		const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(EffectHandle);
		if (ActiveEffect && ActiveEffect->Spec.Def)
		{
			// Get the asset tags from the effect
			FGameplayTagContainer AssetTags;
			ActiveEffect->Spec.GetAllAssetTags(AssetTags);
            
			// Get stack count
			int32 StackCount = ActiveEffect->Spec.GetStackCount();
            
			// Store each tag with its stack count
			for (const FGameplayTag& Tag : AssetTags)
			{
				// Only add tags that match the "Buff" category
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Buff"))))
				{
					Buffs.Add(Tag, StackCount);
				}
			}
		}
	}
    
	return Buffs;
}

TMap<FGameplayTag, int32> ACharacterBase::GetActiveDebuffs() const
{
	TMap<FGameplayTag, int32> Debuffs;

	// early return if ability system component is invalid
	if (!AbilitySystemComponent) return Debuffs;
    
	// Get all active effects
	TArray<FActiveGameplayEffectHandle> ActiveBuffs = AbilitySystemComponent->GetActiveEffectsWithAllTags(DebuffFilterTags);
    
	for (const FActiveGameplayEffectHandle& EffectHandle : ActiveBuffs)
	{
		const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(EffectHandle);
		if (ActiveEffect && ActiveEffect->Spec.Def)
		{
			// Get the asset tags from the effect
			FGameplayTagContainer AssetTags;
			ActiveEffect->Spec.GetAllAssetTags(AssetTags);
            
			// Get stack count
			int32 StackCount = ActiveEffect->Spec.GetStackCount();
            
			// Store each tag with its stack count
			for (const FGameplayTag& Tag : AssetTags)
			{
				// Only add tags that match the "Debuff" category
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Debuff"))))
				{
					Debuffs.Add(Tag, StackCount);
				}
			}
		}
	}
    
	return Debuffs;
}


// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Check if the current persistent level is "L_AbilityDraft"
	UWorld* World = GetWorld();
	if (World && World->GetMapName().EndsWith(TEXT("L_AbilityDraft")))
	{
		// The current level is L_AbilityDraft
		InitialiseAbilities();
	}
	else
	{
		InitialiseAbilities();
		InitialiseEffects();
	}

}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return(AbilitySystemComponent);
}

void ACharacterBase::ActivateAbility(const TSubclassOf<UGameplayAbility> AbilityClass)
{
	// Early exit case if the class is empty
	if (!AbilityClass) return;
	
	AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass, false);
}

void ACharacterBase::ActivateAbilityWithTargets(TSubclassOf<UGameplayAbility> AbilityClass,
	const TArray<AActor*> InTargetsActor)
{
	// Early exit case if the class is empty
	if (!AbilityClass) return;
	
	Targets = InTargetsActor;
	AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass, false);
}

void ACharacterBase::ActivateAbilityTargetingSelf(TSubclassOf<UGameplayAbility> AbilityClass)
{
	const TArray<AActor*> TargetsActor {this};
	if (!AbilityClass) return;

	Targets = TargetsActor;
	AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass, false);
}

void ACharacterBase::InitialiseAbilities()
{
	// Iterate through all abilities in the array and give them to the character
	for (TSubclassOf<UGameplayAbility>& Ability : Abilities)
	{
		if (Ability)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
		}
	}
}

void ACharacterBase::InitialiseEffects()
{
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// Foreach the effect array
	for (TSubclassOf<UGameplayEffect>& Effect: DefaultEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

// Relay the delegate data to the blueprint-usable events
void ACharacterBase::OnDamageTakenChanged(AActor* DamageInstigator, AActor* DamageCauser,
	const FGameplayTagContainer& GameplayTagContainer, float Damage)
{
	OnDamageTaken(DamageInstigator, DamageCauser, GameplayTagContainer, Damage);
}
// Relay the delegate data to the blueprint-usable events
void ACharacterBase::OnCurrentHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnCurrentHealthChanged(Data.OldValue, Data.NewValue);
}

void ACharacterBase::ActivateStartOfTurnEffects()
{
	for (FActiveGameplayEffectHandle Effect : AbilitySystemComponent->GetActiveEffectsWithAllTags(StartFilterTags))
	{
		int32 StacksToRemove = 1;

		if (const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(Effect))
		{
			FGameplayTagContainer AssetTags;
			ActiveEffect->Spec.GetAllAssetTags(AssetTags);

			if (AssetTags.HasTagExact(RemoveAllStartOfTurnStacksTag))
			{
				// -1 means remove all stacks for this active gameplay effect.
				StacksToRemove = -1;
			}
		}

		AbilitySystemComponent->RemoveActiveGameplayEffect(Effect, StacksToRemove);
	}
}

void ACharacterBase::ActivateEndOfTurnEffects()
{
	for (FActiveGameplayEffectHandle Effect : AbilitySystemComponent->GetActiveEffectsWithAllTags(EndFilterTags))
	{
		int32 StacksToRemove = 1;

		if (const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(Effect))
		{
			FGameplayTagContainer AssetTags;
			ActiveEffect->Spec.GetAllAssetTags(AssetTags);

			if (AssetTags.HasTagExact(RemoveAllEndOfTurnStacksTag))
			{
				// -1 means remove all stacks for this active gameplay effect.
				StacksToRemove = -1;
			}
		}

		AbilitySystemComponent->RemoveActiveGameplayEffect(Effect, StacksToRemove);
	}
}

TArray<AActor*> ACharacterBase::GetTargets() const
{
	return Targets;
}

TArray<UGameplayAbilityBase*> ACharacterBase::GetAllAbilityInstances() const
{
	TArray<UGameplayAbilityBase*> AbilityInstances;

	if (!AbilitySystemComponent)
	{
		return AbilityInstances;
	}

	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Ability)
		{
			UGameplayAbilityBase* AbilityInstance = Cast<UGameplayAbilityBase>(Spec.Ability);
			if (AbilityInstance)
			{
				AbilityInstances.Add(AbilityInstance);
			}
		}
	}

	return AbilityInstances;
}

// removes a given number of stacks of an effect that gives that status tag
// returns true if successful and false otherwise
bool ACharacterBase::RemoveStacksOfEffectWithTag(FGameplayTag StatusTag, int NumStacks)
{
	if (!AbilitySystemComponent) return false;
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(StatusTag);

	for (FActiveGameplayEffectHandle Effect : AbilitySystemComponent->GetActiveEffectsWithAllTags(AssetTags))
	{
		int32 StacksToRemove = NumStacks;

		if (const FActiveGameplayEffect* ActiveEffect = AbilitySystemComponent->GetActiveGameplayEffect(Effect))
		{
			// remove all stacks if 'stacks to remove' is more than the number of stacks active
			if (ActiveEffect->Spec.GetStackCount() <= StacksToRemove)
			{
				// -1 means remove all stacks for this active gameplay effect.
				StacksToRemove = -1;
			}
		}

		AbilitySystemComponent->RemoveActiveGameplayEffect(Effect, StacksToRemove);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////// Blueprint-friendly Attribute Getters
float ACharacterBase::GetFlatDamageModifier() const
{
	return(DamageModifiersSet->GetFlatModifier());
}

float ACharacterBase::GetMultiDamageModifier() const
{
	return(DamageModifiersSet->GetMultiModifier());
}

float ACharacterBase::GetCurrentHealth() const
{
	return(HealthSet->GetCurrentHealth());
}

float ACharacterBase::GetMaxHealth() const
{
	return(HealthSet->GetMaxHealth());
}

float ACharacterBase::GetCurrentProtection() const
{
	return(HealthSet->GetCurrentProtection());
}

float ACharacterBase::GetCurrentWard() const
{
	return(HealthSet->GetCurrentWard());
}

int ACharacterBase::GetMaxMovement() const
{
	return(FMath::RoundToInt(TurnActionSet->GetMaxMovement()));
}

int ACharacterBase::GetMaxAttacks() const
{
	return(FMath::RoundToInt(TurnActionSet->GetMaxAttacks()));
}

int ACharacterBase::GetAvailableMovement() const
{
	return(FMath::RoundToInt(TurnActionSet->GetAvailableMovement()));
}

int ACharacterBase::GetAvailableAttacks() const
{
	return(FMath::RoundToInt(TurnActionSet->GetAvailableAttacks()));
}


/////////////////////////////////////////////////////////////////////////// Blueprint-friendly Attribute Setters
void ACharacterBase::SetAvailableAttacks(float NewValue)
{
	TurnActionSet->SetAvailableAttacks(NewValue);
}

void ACharacterBase::SetAvailableMovement(float NewValue)
{
	TurnActionSet->SetAvailableMovement(NewValue);
}

void ACharacterBase::SetMaxAttacks(float NewValue)
{
	TurnActionSet->SetMaxAttacks(NewValue);
}

void ACharacterBase::SetMaxMovement(float NewValue)
{
	TurnActionSet->SetMaxMovement(NewValue);
}