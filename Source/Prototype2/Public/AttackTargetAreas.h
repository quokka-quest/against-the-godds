#pragma once

#include "GlobalDataTypeHeader.h"

class AttackTargetAreas
{
public:
	TArray<FIntVector> GetCoordsInTargetArea(FIntVector TargetCoord, EAttackPattern Pattern);
};
