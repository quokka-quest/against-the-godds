// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerEntity.h"

APlayerEntity::APlayerEntity()
{
	CharMesh = GetCharMesh();
	CharacterMesh->SetStaticMesh(CharMesh);
}
