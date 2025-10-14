// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AttributeBaseSet.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UAttributeBaseSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const;
	
};
