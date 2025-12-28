// Fill out your copyright notice in the Description page of Project Settings.


#include "GridOutlineActor.h"

// Sets default values
AGridOutlineActor::AGridOutlineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	SetRootComponent(ProceduralMesh);

	ProceduralMesh->bUseAsyncCooking = true;
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGridOutlineActor::BuildOutlineMesh(const TArray<FOutlineEdge>& StartEndMap, float LineWidth)
{
	TArray<FVector> Verts;
	TArray<int32> Tris;
	TArray<FVector> Norms;
	TArray<FVector2D> UVs;

	float HalfWidth = LineWidth * 0.5f;

	int counter = 0;
	for (FOutlineEdge LineInfo : StartEndMap)
	{
		//if (counter > 0) continue;
		AddEdgeQuad(LineInfo, HalfWidth, Verts, Tris, Norms, UVs);
		counter++;
	}

	ProceduralMesh->ClearAllMeshSections();
	ProceduralMesh->CreateMeshSection(0, Verts, Tris, Norms, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	ProceduralMesh->SetMaterial(0, GridMaterial);
}


// NOTE: something is going wrong in here??? (definitly a winding order and a miter offset direction issue)
void AGridOutlineActor::AddEdgeQuad(const FOutlineEdge& LineInfo, float HalfWidth,
	TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs)
{
	// direction and perpendicular
	FVector Dir = (LineInfo.End - LineInfo.Start).GetSafeNormal();
	FVector Perp = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();

	FVector IncomingOffset = LineInfo.StartMiter;
	FVector OutgoingOffset = LineInfo.EndMiter;

	if (IncomingOffset == FVector::ZeroVector) IncomingOffset = Perp * HalfWidth;
	if (OutgoingOffset == FVector::ZeroVector) OutgoingOffset = Perp * HalfWidth;
	
	int32 IndexStart = Verts.Num();

	/*	adds the vertices to the 'Verts' array making a quad with the below shape:
	 *	          <-Len->
	 *	         A-------B         /\
	 *	Start--> |       | <--End  || Width
	 *	         D-------C         \/
	 */

	Verts.Add(LineInfo.Start+IncomingOffset); // vertex A
	Verts.Add(LineInfo.End+OutgoingOffset);   // Vertex B
	Verts.Add(LineInfo.End-OutgoingOffset);   // Vertex C
	Verts.Add(LineInfo.Start-IncomingOffset); // Vertex D

	UE_LOG(LogTemp, Warning, TEXT("Start Miter: %f, %f"), IncomingOffset.X, IncomingOffset.Y)
	UE_LOG(LogTemp, Warning, TEXT("Vert A pos: %f, %f"), (LineInfo.Start+IncomingOffset).X, (LineInfo.Start+IncomingOffset).Y)
	UE_LOG(LogTemp, Warning, TEXT("Vert B pos: %f, %f"), (LineInfo.End+OutgoingOffset).X, (LineInfo.End+OutgoingOffset).Y)
	UE_LOG(LogTemp, Warning, TEXT("Vert C pos: %f, %f"), (LineInfo.End-OutgoingOffset).X, (LineInfo.End-OutgoingOffset).Y)
	UE_LOG(LogTemp, Warning, TEXT("Vert D pos: %f, %f"), (LineInfo.Start-IncomingOffset).X, (LineInfo.Start-IncomingOffset).Y)

	// Adds the ABC triangle
	Tris.Add(IndexStart+0);
	Tris.Add(IndexStart+2);
	Tris.Add(IndexStart+1);

	// Adds the ACD triangle
	Tris.Add(IndexStart+0);
	Tris.Add(IndexStart+3);
	Tris.Add(IndexStart+2);

	// add the 4 normals for the vertices (assumes quad is laid flat on the XY plane)
	for (int i = 0; i < 4; i++) { Norms.Add(FVector::UpVector); }

	// add simple UVs
	UVs.Add(FVector2D(0,0));
	UVs.Add(FVector2D(1,0));
	UVs.Add(FVector2D(1,1));
	UVs.Add(FVector2D(0,1));
}

void AGridOutlineActor::SetVisibility(bool IsVisible)
{
	ProceduralMesh->SetVisibility(IsVisible);
}
