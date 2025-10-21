#pragma once

#include "CoreMinimal.h"
#include "MapRoomType.generated.h"

UENUM(BlueprintType)
enum class EMapRoomCPP : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Non_Combat UMETA(DisplayName = "Non-Combat"),
	Shop UMETA(DisplayName = "Shop"),
	Boss UMETA(DisplayName = "Boss"),
	Empty UMETA(DisplayName = "Empty")
};