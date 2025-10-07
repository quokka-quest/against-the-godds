// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "HealthAttributeSet.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		HealthAttributeSetComponent = AbilitySystemComponent->GetSet<UHealthAttributeSet>();

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributeSetComponent->GetCurrentHealthAttribute()).AddUObject(this, &ABaseCharacter::OnHealthChangedNative);
	}
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::InitialiseAbility(TSubclassOf<UGameplayAbility> Ability, int32 AbilityLevel)
{
	if (AbilitySystemComponent && Ability)
	{
		if (HasAuthority())
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, AbilityLevel));
			AbilitySystemComponent->InitAbilityActorInfo(this, this);
		}

		//AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ABaseCharacter::GetHealthValues(float& CurrentHealth, float& MaxHealth) const
{
	CurrentHealth = HealthAttributeSetComponent->GetCurrentHealth();
	MaxHealth = HealthAttributeSetComponent->GetMaxHealth();
}

void ABaseCharacter::OnHealthChangedNative(const FOnAttributeChangeData& Data)
{
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

