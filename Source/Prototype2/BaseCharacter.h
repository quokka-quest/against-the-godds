// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "BaseCharacter.generated.h"

class UHealthAttributeSet;

UCLASS()
class PROTOTYPE2_API ABaseCharacter : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// UPROPERTIES

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Abilities")
	const class UHealthAttributeSet* HealthAttributeSetComponent;


	// UFUNCTIONS

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void InitialiseAbility(TSubclassOf<UGameplayAbility> Ability, int32 AbilityLevel);

	UFUNCTION(BlueprintPure, Category = "Abilities")
	void GetHealthValues(float& CurrentHealth, float& MaxHealth) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Abilities")
	void OnHealthChanged(float OldValue, float NewValue);




	void OnHealthChangedNative(const FOnAttributeChangeData& Data);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

};
