// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GridOutlineActor.generated.h"

struct FLineInfo
{
	FVector Start;
	FVector End;
	FVector IncomingMiter;
	FVector OutgoingMiter;
};

UCLASS()
class PROTOTYPE2_API AGridOutlineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridOutlineActor();

	void BuildOutlineMesh(const TMap<FVector, FVector>& StartEndMap, float LineWidth);

	void SetMaterial(UMaterialInterface* Mat) { GridMaterial = Mat; }

protected:
	UPROPERTY()
	UProceduralMeshComponent* ProceduralMesh;

	void AddEdgeQuad(const FLineInfo& LineInfo, float HalfWidth,
		TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs);

	void CalculateMiterOffsets(TMap<FVector, FLineInfo>& OutLineMap, const TMap<FVector, FVector>& StartEndMap, const float HalfWidth);

	UPROPERTY()
	UMaterialInterface* GridMaterial;

};
