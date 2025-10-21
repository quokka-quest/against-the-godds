#pragma once

#include "CoreMinimal.h"
#include "MapRoomType.generated.h"

UENUM(BlueprintType)
enum class EMapRoomType : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Non_Combat UMETA(DisplayName = "Non-Combat"),
	Rest UMETA(DisplayName = "Rest"),
	Boss UMETA(DisplayName = "Boss")
};