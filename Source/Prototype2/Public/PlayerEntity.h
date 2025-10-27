// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntityBase.h"
#include "PlayerEntity.generated.h"

/**
 * 
 */
UCLASS()
class PROTOTYPE2_API APlayerEntity : public AEntityBase
{
	GENERATED_BODY()
public:
	APlayerEntity();

protected:
	virtual UStaticMesh* GetCharMesh() const { return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Static_Meshes/PlayerCharacters/SM_Fighter.SM_Fighter")); }
};
