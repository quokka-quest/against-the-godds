// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseAttributeSet.h"
#include "HealthAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API UHealthAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

public:
	// Current Health of the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Attributes")
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, CurrentHealth)

	// Maximum Health of the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth)

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

};
