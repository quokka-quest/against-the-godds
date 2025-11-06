#pragma once

#include "GlobalDataTypeHeader.h"

class AttackTargetAreas
{
public:
	AttackTargetAreas();
	
	TArray<FIntVector> GetCoordsInTargetArea(FIntVector TargetCoord, EAttackPattern Pattern, EAttackRotation Rotation);

private:
	TArray<FIntVector> GetPatternToUse(EAttackPattern Pattern);
	EAttackRotation AttackRotation;

	TArray<FIntVector> GetCentredLine3X1();
	TArray<FIntVector> GetEndLine3X1();
	
	TArray<FIntVector> SingleTarget =
	{
		FIntVector(0,0,0)
	};

	TArray<FIntVector> Plus =
	{
		FIntVector(0,0,0),
		FIntVector(1,0,0),
		FIntVector(-1,0,0),
		FIntVector(0,1,0),
		FIntVector(0,-1,0)
	};
};
