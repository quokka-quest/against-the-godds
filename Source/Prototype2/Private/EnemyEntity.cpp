// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyEntity.h"

AEnemyEntity::AEnemyEntity()
{
	CharMesh = GetCharMesh();
	CharacterMesh->SetStaticMesh(CharMesh);
}
