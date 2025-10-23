// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "EntityBase.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API AEntityBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEntityBase();

protected:
	UPROPERTY()
	UStaticMeshComponent* BaseplateMesh;
	UPROPERTY()
	UStaticMeshComponent* CharacterMesh;

	UPROPERTY()
	UStaticMesh* BaseMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Static_Meshes/Bases/SM_Standard_Base.SM_Standard_Base"));
	UPROPERTY()
	UStaticMesh* CharMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Levels/_GENERATED/kibbl/SM_PlayerRef.SM_PlayerRef"));
};
