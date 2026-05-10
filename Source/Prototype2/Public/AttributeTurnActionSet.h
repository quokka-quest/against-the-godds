// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeBaseSet.h"
#include "AbilitySystemComponent.h"
#include "AttributeTurnActionSet.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UAttributeTurnActionSet : public UAttributeBaseSet
{
	GENERATED_BODY()

public:
	UAttributeTurnActionSet();

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AvailableMovement;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeTurnActionSet, AvailableMovement);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxMovement;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeTurnActionSet, MaxMovement);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AvailableAttacks;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeTurnActionSet, AvailableAttacks);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAttacks;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeTurnActionSet, MaxAttacks);

public:
	virtual void ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
};
