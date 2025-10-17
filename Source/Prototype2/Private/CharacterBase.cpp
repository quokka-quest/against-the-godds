// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AttributeHealthSet.h"
#include "AttributeDamageModifiersSet.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the GAS components
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	HealthSet = CreateDefaultSubobject<UAttributeHealthSet>(TEXT("HealthSet"));
	DamageModifiersSet = CreateDefaultSubobject<UAttributeDamageModifiersSet>(TEXT("DamageModifiersSet"));
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

void ACharacterBase::ActivateAbilityWithTarget(TSubclassOf<UGameplayAbility> AbilityClass, AActor* InTargetActor)
{
	// Early exit case if the class is empty
	if (!AbilityClass) return;
	
	TargetActor = InTargetActor;
	AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass, false);
}

void ACharacterBase::InitialiseAbilities()
{
	// If AbilityOne is not null, add it to the character
	if (AbilityOne)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityOne));
	}
	// If AbilityTwo is not null, add it to the character
	if (AbilityTwo)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityTwo));
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

float ACharacterBase::GetFlatDamageModifier() const
{
	return(DamageModifiersSet->GetFlatModifier());
}

float ACharacterBase::GetMultiDamageModifier() const
{
	return(DamageModifiersSet->GetMultiModifier());
}

