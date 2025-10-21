// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapRoomType.h"
#include "Engine/GameInstance.h"
#include "GameManager.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMapNodeData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	EMapRoomType RoomType;

	UPROPERTY(EditANywhere, BlueprintReadWrite, Category = "Map")
	bool bVisited = false;

	FMapNodeData()
		: RoomType(EMapRoomType::Combat), bVisited(false)
	{}
};

UCLASS()
class PROTOTYPE2_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	TArray<EMapRoomType> GenerateMap();
	void ShuffleArray(TArray<EMapRoomType>& ArrayToShuffle);

private:
	TArray<EMapRoomType> AllRooms;
	TArray<FMapNodeData> MapNodes;
	FMapNodeData MapDataStruct;
	int TotalRooms = 10;
	int RestCount = TotalRooms / 5;
	int CombatCount = (TotalRooms - RestCount) / 2;
	int NonCombatCount = (TotalRooms - RestCount) / 2;
	int CurrentNodeIndex = 0;

	
};
