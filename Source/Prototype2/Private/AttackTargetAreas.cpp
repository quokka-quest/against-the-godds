#include "AttackTargetAreas.h"

TArray<FIntVector> AttackTargetAreas::GetCoordsInTargetArea(FIntVector TargetCoord, EAttackPattern Pattern)
{
	TArray<FIntVector> Result;
	TArray<FIntVector> Offsets = GetPatternToUse(Pattern);
	if (Offsets.Num() <= 0) return Result;

	for (int i = 0; i < Offsets.Num(); i++)
	{
		Result.Add(Offsets[i] + TargetCoord);
	}

	return Result;
}

TArray<FIntVector> AttackTargetAreas::GetPatternToUse(EAttackPattern Pattern)
{
	if (Pattern == EAttackPattern::SingleTarget) return SingleTarget;
	if (Pattern == EAttackPattern::Plus) return Plus;
	if (Pattern == EAttackPattern::XCentredLine3X1) return XCentredLine3X1;
	if (Pattern == EAttackPattern::YCentredLine3X1) return YCentredLine3X1;
	
	return TArray<FIntVector>();
}
