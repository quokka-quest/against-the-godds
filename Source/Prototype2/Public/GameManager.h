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
	EMapRoomCPP RoomType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	bool bVisited;

	FMapNodeData()
		: RoomType(EMapRoomCPP::Empty), bVisited(false)
	{}
};

UCLASS()
class PROTOTYPE2_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	TArray<FMapNodeData> GenerateMap();
	void ShuffleArray(TArray<EMapRoomCPP>& ArrayToShuffle);

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	TArray<FMapNodeData> GetMapNodes() const { return MapNodes; }

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	int GetCurrentNodeIndex() const { return CurrentNodeIndex; }

	UFUNCTION(BlueprintCallable, Category="GameManager")
	void ModifyArrayStruct(bool bVisitedStatus, int ArrayIndex);

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	bool isEncounterComplete(FName EncounterName) const;

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void MarkEncounterComplete(FName EncounterName);

private:
	TArray<EMapRoomCPP> AllRooms;
	
	int TotalRooms = 10;
	int RestCount = TotalRooms / 5;
	int CombatCount = (TotalRooms - RestCount) / 2;
	int NonCombatCount = (TotalRooms - RestCount) / 2;

protected:
	UPROPERTY(BlueprintReadWrite, Category="Map")
	int CurrentNodeIndex = 0;
	FMapNodeData MapDataStruct;
	UPROPERTY(BlueprintReadWrite, Category="Map")
	TArray<FMapNodeData> MapNodes;

	UPROPERTY(BlueprintReadWrite, Category = "Map")
	TArray<FName> CompletedEncounters;

	
};
