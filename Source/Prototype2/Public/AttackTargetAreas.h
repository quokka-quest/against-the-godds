#pragma once

#include "GlobalDataTypeHeader.h"

class AttackTargetAreas
{
public:
	TArray<FIntVector> GetCoordsInTargetArea(FIntVector TargetCoord, EAttackPattern Pattern);

private:
	TArray<FIntVector> GetPatternToUse(EAttackPattern Pattern);
	
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

	TArray<FIntVector> XCentredLine3X1 =
	{
		FIntVector(0,0,0),
		FIntVector(1,0,0),
		FIntVector(-1,0,0)
	};

	TArray<FIntVector> YCentredLine3X1 =
	{
		FIntVector(0,0,0),
		FIntVector(0,1,0),
		FIntVector(0,-1,0)
	};
};
