// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeBaseSet.h"

void UAttributeBaseSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	ClampAttributeOnChange(Attribute, NewValue);
}

void UAttributeBaseSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	ClampAttributeOnChange(Attribute, NewValue);
}

// Override this function when implementing new attribute sets that require clamping the values
void UAttributeBaseSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
}
