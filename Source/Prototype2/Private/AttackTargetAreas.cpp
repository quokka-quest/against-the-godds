#include "AttackTargetAreas.h"

AttackTargetAreas::AttackTargetAreas()
{
	AttackRotation = EAttackRotation::R0;
}


TArray<FIntVector> AttackTargetAreas::GetCoordsInTargetArea(FIntVector TargetCoord, EAttackPattern Pattern, EAttackRotation Rotation)
{
	AttackRotation = Rotation;
	
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
	if (Pattern == EAttackPattern::CentredLine3X1) return GetCentredLine3X1();
	if (Pattern == EAttackPattern::EndLine3X1) return GetEndLine3X1();
	
	return TArray<FIntVector>();
}

TArray<FIntVector> AttackTargetAreas::GetCentredLine3X1()
{
	TArray<FIntVector> Result;
	int XVal = (AttackRotation == R0 || AttackRotation == R180)? 1: 0;
	int YVal = (AttackRotation == R90 || AttackRotation == R270)? 1: 0;
	
	Result.Add(FIntVector(0,0,0));
	Result.Add(FIntVector(XVal, YVal, 0));
	Result.Add(FIntVector(-XVal, -YVal, 0));
	
	return Result;
}

TArray<FIntVector> AttackTargetAreas::GetEndLine3X1()
{
	TArray<FIntVector> Result;
	int XVal = (AttackRotation == R0)? 1: (AttackRotation == R180)? -1: 0;
	int YVal = (AttackRotation == R90)? 1: (AttackRotation == R270)? -1: 0;

	Result.Add(FIntVector(0,0,0));
	Result.Add(FIntVector(XVal, YVal, 0));
	Result.Add(FIntVector(XVal*2, YVal*2, 0));

	return Result;
}
