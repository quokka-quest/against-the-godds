// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeBaseSet.h"
#include "AbilitySystemComponent.h"
#include "AttributeDamageModifiersSet.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UAttributeDamageModifiersSet : public UAttributeBaseSet
{
	GENERATED_BODY()

public:
	UAttributeDamageModifiersSet();
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData FlatModifier;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeDamageModifiersSet, FlatModifier);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MultiModifier;
	ATTRIBUTE_ACCESSORS_BASIC(UAttributeDamageModifiersSet, MultiModifier);
};
