// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeHealthSet.h"
#include "EntityBase.h"
#include "GameplayEffectExtension.h"

UAttributeHealthSet::UAttributeHealthSet()
{
	InitMaxHealth(100);
	InitCurrentHealth(100);
}

void UAttributeHealthSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UAttributeHealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	// If we are doing damage
	if (Data.EvaluatedData.Attribute == GetInDamageAttribute())
	{
		// Retrieve the damage done and then set it back to zero
		float InDamageDone = GetInDamage(); // Not making it a const incase we add a shield/armour system
		SetInDamage(0);

		// Check if we are doing any damage first
		if (InDamageDone > 0)
		{
			// if the delegate is bound
			if (OnDamageTaken.IsBound())
			{
				// Retrieve the effect applied and get its data such as magnitude etc and broadcast it to the delegate
				const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
				AActor* Instigator = EffectContext.GetOriginalInstigator();
				AActor* Causer = EffectContext.GetEffectCauser();

				OnDamageTaken.Broadcast(Instigator, Causer, Data.EffectSpec.CapturedSourceTags.GetSpecTags(), Data.EvaluatedData.Magnitude);
			}

			// If the health attribute isn't 0, apply the damage
			if (GetCurrentHealth() > 0)
			{
				const float NewHealth = GetCurrentHealth() - InDamageDone;
				SetCurrentHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
			}

			// check for death
			if (GetCurrentHealth() <= 0)
			{
				Cast<AEntityBase>(GetOwningActor())->OnEntityDeath();
			}
		}
	}
}
