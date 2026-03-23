// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeBaseSet.h"
#include "AbilitySystemComponent.h"
#include "AttributeHealthSet.generated.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FDamageTakenEvent, AActor* /* Effect Instigator*/, AActor* /* EffectCauser */, const FGameplayTagContainer& /* GameplayTagContainer */, float DamageToApply /* The amount of Damage */)

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UAttributeHealthSet : public UAttributeBaseSet
{
	GENERATED_BODY()

public:
	UAttributeHealthSet();

	// Members
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeHealthSet, CurrentHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeHealthSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData InDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeHealthSet, InDamage);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CurrentWard;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeHealthSet, CurrentWard);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CurrentProtection;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeHealthSet, CurrentProtection);

	mutable FDamageTakenEvent OnDamageTaken;

	// Methods
public:
	virtual void ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
