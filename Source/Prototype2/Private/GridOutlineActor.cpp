// Fill out your copyright notice in the Description page of Project Settings.


#include "GridOutlineActor.h"

#include "Chaos/Collision/ConvexFeature.h"

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

void AGridOutlineActor::BuildOutlineMesh(const TMap<FVector, FVector>& StartEndMap, float LineWidth)
{
	TArray<FVector> Verts;
	TArray<int32> Tris;
	TArray<FVector> Norms;
	TArray<FVector2D> UVs;

	float HalfWidth = LineWidth * 0.5f;

	TMap<FVector, FLineInfo> Lines;
	CalculateMiterOffsets(Lines, StartEndMap, HalfWidth);

	for (auto LineInfo : Lines)
	{
		AddEdgeQuad(LineInfo.Value, HalfWidth, Verts, Tris, Norms, UVs);
	}

	ProceduralMesh->ClearAllMeshSections();
	ProceduralMesh->CreateMeshSection(0, Verts, Tris, Norms, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	ProceduralMesh->SetMaterial(0, GridMaterial);
}

void AGridOutlineActor::CalculateMiterOffsets(TMap<FVector, FLineInfo>& OutLineMap, const TMap<FVector, FVector>& StartEndMap, const float HalfWidth)
{
	for (auto Line : StartEndMap)
	{
		// safety error message to prevent Unreal crashing
		if (!StartEndMap.Contains(Line.Value))
		{UE_LOG(LogTemp, Error, TEXT("GridOutlineActor->CalculateMiterOffsets: StartEndMap does not contain a closed loop")) return;}
		
		// calculate the direction of the current line and the next line
		FVector Dir = (Line.Value - Line.Key).GetSafeNormal();
		FVector NextDir = (StartEndMap[Line.Value] - Line.Value).GetSafeNormal();
		// calculate their perpendicular vectors
		FVector Perp = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();
		FVector NextPerp = FVector::CrossProduct(NextDir, FVector::UpVector).GetSafeNormal();

		// calculate the Miter offset
		FVector MiterDir = (Perp + NextPerp).GetSafeNormal();
		float MiterLength = HalfWidth / FVector::DotProduct(MiterDir, NextPerp);
		FVector Offset = MiterDir * MiterLength;

		// set the outgoing Miter for the current line
		// if the current line already exists in the output then set its value. Otherwise, add it to the map
		if (OutLineMap.Contains(Line.Key)) { OutLineMap[Line.Key].OutgoingMiter = Offset; }
		else
		{
			FLineInfo LineInfo;
			LineInfo.Start = Line.Key;
			LineInfo.End = Line.Value;
			LineInfo.OutgoingMiter = Offset;
			OutLineMap.Add(Line.Key, LineInfo);
		}

		// set the incoming miter of the next lines
		// if the line already exists in the output map then set its value. Otherwise, add it to the map
		if (OutLineMap.Contains(Line.Value)) { OutLineMap[Line.Value].IncomingMiter = Offset; }
		else
		{
			FLineInfo NextLineInfo;
			NextLineInfo.Start = Line.Value;
			NextLineInfo.End = StartEndMap[Line.Value];
			NextLineInfo.IncomingMiter = Offset;
			OutLineMap.Add(Line.Value, NextLineInfo);
		}
	}
}

void AGridOutlineActor::AddEdgeQuad(const FLineInfo& LineInfo, float HalfWidth,
	TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs)
{
	// direction and perpendicular
	FVector Dir = (LineInfo.End - LineInfo.Start).GetSafeNormal();
	FVector Perp = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();

	FVector IncomingOffset = LineInfo.IncomingMiter;
	FVector OutgoingOffset = LineInfo.OutgoingMiter;
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
