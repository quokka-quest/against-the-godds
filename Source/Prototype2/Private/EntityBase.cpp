// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityBase.h"

#include "AttributeHealthSet.h"

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

void AEntityBase::PrintDebugData()
{
	UE_LOG(LogTemp, Warning, TEXT("Current Health: %f"), HealthSet->GetCurrentHealth());
}
