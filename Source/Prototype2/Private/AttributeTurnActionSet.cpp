// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeTurnActionSet.h"

UAttributeTurnActionSet::UAttributeTurnActionSet()
{
	InitMaxMovement(4);
	InitMaxAttacks(2);
	InitAvailableMovement(GetMaxMovement());
	InitAvailableAttacks(GetMaxAttacks());
}

void UAttributeTurnActionSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetAvailableMovementAttribute() || Attribute == GetAvailableAttacksAttribute()) NewValue = FMath::Max(0.0f, NewValue);
}
