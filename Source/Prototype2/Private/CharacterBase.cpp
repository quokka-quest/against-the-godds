// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AttributeHealthSet.h"
#include "AttributeDamageModifiersSet.h"
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
	StartFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Effect.StartOfTurn")));
	StatusFilterTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status")));
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

	InitialiseEffects();
	InitialiseAbilities();
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

float ACharacterBase::GetCurrentHealth() const
{
	return(HealthSet->GetCurrentHealth());
}

float ACharacterBase::GetMaxHealth() const
{
	return(HealthSet->GetMaxHealth());
}

void ACharacterBase::ActivateStartOfTurnEffects()
{
	for (FActiveGameplayEffectHandle Effect : AbilitySystemComponent->GetActiveEffectsWithAllTags(StartFilterTags))
	{
		// Remove a stack
		AbilitySystemComponent->RemoveActiveGameplayEffect(Effect, 1);
	}
}

float ACharacterBase::GetFlatDamageModifier() const
{
	return(DamageModifiersSet->GetFlatModifier());
}

float ACharacterBase::GetMultiDamageModifier() const
{
	return(DamageModifiersSet->GetMultiModifier());
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

