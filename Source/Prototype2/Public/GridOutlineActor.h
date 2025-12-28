// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GridOutlineActor.generated.h"

struct FOutlineEdge
{
	FVector Start;
	FVector End;

	FVector StartMiter;
	FVector EndMiter;

	bool operator==(const FOutlineEdge& Other) const
	{
		return (Start == Other.Start && End == Other.End);
	}
};

struct FOutlineVertexInfo
{
	TArray<FOutlineEdge> Edges;
};

UCLASS()
class PROTOTYPE2_API AGridOutlineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridOutlineActor();

	void BuildOutlineMesh(const TArray<FOutlineEdge>& StartEndMap, float LineWidth);

	void SetMaterial(UMaterialInterface* Mat) { GridMaterial = Mat; }

	void SetVisibility(bool IsVisible);

protected:
	UPROPERTY()
	UProceduralMeshComponent* ProceduralMesh;

	void AddEdgeQuad(const FOutlineEdge& LineInfo, float HalfWidth,
		TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs);

	UPROPERTY()
	UMaterialInterface* GridMaterial;

};
