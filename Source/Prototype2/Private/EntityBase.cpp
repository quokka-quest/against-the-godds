// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityBase.h"

AEntityBase::AEntityBase()
{
	BaseplateMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseplateMesh");
	RootComponent = BaseplateMesh;

	CharacterMesh = CreateDefaultSubobject<UStaticMeshComponent>("CharacterMesh");
	CharacterMesh->SetupAttachment(BaseplateMesh);

	if (BaseMesh) BaseplateMesh->SetStaticMesh(BaseMesh);
	if (CharMesh) CharacterMesh->SetStaticMesh(CharMesh);

	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseplateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


