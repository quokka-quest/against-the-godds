// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeHealthSet.h"
#include "EntityBase.h"
#include "GameplayEffectExtension.h"

UAttributeHealthSet::UAttributeHealthSet()
{
	InitMaxHealth(100);
	InitCurrentHealth(100);
	InitCurrentProtection(0);
	InitCurrentWard(0);
}

void UAttributeHealthSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetCurrentWardAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetCurrentProtectionAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}

void UAttributeHealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	// If we are doing damage
	if (Data.EvaluatedData.Attribute == GetInDamageAttribute())
	{
		float RemainingDamage = GetInDamage();
		SetInDamage(0);

		// Check if we are doing any damage first
		if (RemainingDamage > 0)
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

			if (GetCurrentProtection() <= 0)
			{
				if (GetCurrentWard() > 0 && RemainingDamage > 0)
				{
					const float WardAbsorbed = FMath::Min(GetCurrentWard(), RemainingDamage);
					SetCurrentWard(GetCurrentWard() - WardAbsorbed);
					RemainingDamage -= WardAbsorbed;
				}

				// Any damage left after ward hits health.
				if (GetCurrentHealth() > 0 && RemainingDamage > 0)
				{
					const float NewHealth = GetCurrentHealth() - RemainingDamage;
					SetCurrentHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
				}

				// check for death
				if (GetCurrentHealth() <= 0)
				{
					if (AEntityBase* Entity = Cast<AEntityBase>(GetOwningActor()))
					{
						Entity->OnEntityDeath();
					}
				}
			}
			else
			{
				SetCurrentProtection(GetCurrentProtection() - 1);
			}
		}
	}
}
