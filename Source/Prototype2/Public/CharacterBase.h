// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "TurnBasedAbilitySystemComponent.h"
#include "CharacterBase.generated.h"

//Forward declarations for included classes inside the cpp file
class UGameplayAbility;
class UGameplayEffect;

UCLASS()
class PROTOTYPE2_API ACharacterBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

private:
	FGameplayTagContainer StartFilterTags;
	FGameplayTagContainer StatusFilterTags;

public:
	// Sets default values for this pawn's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// GAS Setup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTurnBasedAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAttributeHealthSet> HealthSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAttributeDamageModifiersSet> DamageModifiersSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	UFUNCTION(BlueprintCallable, Category = "Character|Status Effects")
    TMap<FGameplayTag, int32> GetActiveStatusEffects() const;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void ActivateAbility(TSubclassOf<UGameplayAbility> AbilityClass);
	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void ActivateAbilityWithTargets(TSubclassOf<UGameplayAbility> AbilityClass, const TArray<AActor*> InTargetsActor);
	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void ActivateAbilityTargetingSelf(TSubclassOf<UGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetFlatDamageModifier() const;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetMultiDamageModifier() const;

	TArray<AActor*> GetTargets() const;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	TArray<UGameplayAbilityBase*> GetAllAbilityInstances() const;
	
protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(BlueprintReadOnly, Category = "GAS")
	TArray<AActor*> Targets;

	virtual void InitialiseAbilities();
	virtual void InitialiseEffects();

	virtual void OnDamageTakenChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayTagContainer& GameplayTagContainer, float Damage);

	UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
	void OnDamageTaken(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayTagContainer& GameplayTagContainer, float Damage);

	virtual void OnCurrentHealthAttributeChanged(const FOnAttributeChangeData& Data);

	UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
	void OnCurrentHealthChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetCurrentProtection() const;
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetCurrentWard() const;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	void ActivateStartOfTurnEffects();
	
};
