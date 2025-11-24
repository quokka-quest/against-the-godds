// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapRoomType.h"
#include "Delegates/DelegateCombinations.h"
#include "Engine/GameInstance.h"
#include "GameManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentNodeChanged, const TArray<int32>&, ConnectedNodeIndices);

USTRUCT(BlueprintType)
struct FMapNodeData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	EMapRoomCPP RoomType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	bool bVisited;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float positionX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float positionY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TArray<int32> ConnectedNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TArray<int32> IncomingNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 column = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int32 row = -1;

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
	TArray<FMapNodeData> GetMapNodes() const { return MapNodes; }

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	int GetCurrentNodeIndex() const { return CurrentNodeIndex; }

	UFUNCTION(BlueprintCallable, Category = "GameManager")
	void GenerateGrid();

	UFUNCTION(BlueprintCallable, Category = "Map")
	void CreateMap();

	void CreateMapPaths();

	void ChooseStartingNodes();

	bool DoesConnectionCross(int32 Floor, int32 ColA, int32 ColB);

	UFUNCTION(BlueprintCallable, Category = "Map")
	TArray<int32> ChooseNextNodes(int32 Floor, int32 NodeIndex);

	UFUNCTION(BlueprintCallable, Category = "Map")
	FMapNodeData GetNode(int32 Floor, int32 NodeIndex);

	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetNode(int32 Floor, int32 NodeIndex, FMapNodeData& NewData);

	UFUNCTION(BlueprintCallable, Category = "Map")
	int32 GetNodeIndex(int32 Floor, int32 NodeIndex);

	UFUNCTION(BlueprintCallable, Category = "Map")
	void setCurrentNodePosition(int32 node, float x, float y);

	UPROPERTY(BlueprintAssignable, Category = "Map")
	FOnCurrentNodeChanged OnCurrentNodeChanged;
    
	UFUNCTION(BlueprintCallable, Category = "Map")
	bool TryMoveToNode(int32 TargetNodeIndex);

	UFUNCTION(BlueprintCallable, Category = "Map")
	void BroadcastCurrentNodeConnections();

	UFUNCTION(BlueprintCallable, Category = "Map")
	bool isEncounterCompleted(FName EncounterName);

private:
	TArray<EMapRoomCPP> AllRooms;

protected:
	UPROPERTY(BlueprintReadWrite, Category="Map")
	int CurrentNodeIndex = -1;
	UPROPERTY(BlueprintReadWrite, Category = "Map")
	bool bMapGenerated = false;
	FMapNodeData MapDataStruct;
	UPROPERTY(BlueprintReadWrite, Category="Map")
	TArray<FMapNodeData> MapNodes;

	UPROPERTY(BlueprintReadWrite, Category = "Map")
	TArray<FName> CompletedEncounters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int floors = 15;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	int maxNodesPerFloor = 7;

	int TotalRooms = floors * maxNodesPerFloor;
	int RestCount = TotalRooms / 5;
	int CombatCount = (TotalRooms - RestCount) / 2;
	int NonCombatCount = (TotalRooms - RestCount) / 2;

	UPROPERTY(BlueprintReadWrite, Category = "Map")
	TArray<FMapNodeData> Grid;

	EMapRoomCPP DefaultRoomType = EMapRoomCPP::Empty;
	EMapRoomCPP Selected = EMapRoomCPP::Selected;

	TArray<int> StartingNodes;
	
};
