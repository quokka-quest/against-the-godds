// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "CharacterBase.generated.h"

//Forward declarations for included classes inside the cpp file
class UGameplayAbility;
class UGameplayEffect;

UCLASS()
class PROTOTYPE2_API ACharacterBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// GAS Setup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAttributeHealthSet> HealthSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAttributeDamageModifiersSet> DamageModifiersSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> AbilityOne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> AbilityTwo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void ActivateAbility(TSubclassOf<UGameplayAbility> AbilityClass);
	UFUNCTION(BlueprintCallable, Category = "GAS")
	virtual void ActivateAbilityWithTarget(TSubclassOf<UGameplayAbility> AbilityClass, AActor* InTargetActor);

	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetFlatDamageModifier() const;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	float GetMultiDamageModifier() const;
protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(BlueprintReadOnly, Category = "GAS")
	AActor* TargetActor;

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

	
};
