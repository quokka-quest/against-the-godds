// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"

TArray<FMapNodeData> UGameManager::GenerateMap()
{
	if(AllRooms.Num() == 0)
	{
		// Populate AllRooms with the required number of each room type
		for(int i = 0; i < CombatCount; i++)
		{
			AllRooms.Add(EMapRoomCPP::Combat);
			//UE_LOG(LogTemp, Warning, TEXT("Added Combat Room %d"), i);
		}
		for(int i = 0; i < NonCombatCount; i++)
		{
			AllRooms.Add(EMapRoomCPP::Non_Combat);
			//UE_LOG(LogTemp, Warning, TEXT("Added NonCombat Room %d"), i);
		}
		for(int i = 0; i < RestCount; i++)
		{
			AllRooms.Add(EMapRoomCPP::Shop);
			//UE_LOG(LogTemp, Warning, TEXT("Added Shop Room %d"), i);
		}

		// Shuffle the rooms to randomize their order
		ShuffleArray(AllRooms);

		for (int i = 0; i < AllRooms.Num(); i++)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Shuffled Room %d: %d"), i, (AllRooms[i]));
		}

		// Add the guaranteed Rest and Boss rooms at the end
		AllRooms.Add(EMapRoomCPP::Shop);
		AllRooms.Add(EMapRoomCPP::Boss);

		for (int i = 0; i < AllRooms.Num(); i++)
		{
			MapDataStruct.RoomType = AllRooms[i];
			MapNodes.Add(MapDataStruct);
		}

		for(int i = 0; i < MapNodes.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("Map Node %d: %d"), i, (int)MapNodes[i].RoomType);
		}

		// Set the first room as visited
		MapNodes[0].bVisited = true;
		return MapNodes;
	}
	else
	{
		return MapNodes;
	}
}

void UGameManager::ShuffleArray(TArray<EMapRoomCPP>& ArrayToShuffle)
{
	int32 LastIndex = ArrayToShuffle.Num() - 1;
	for (int32 i = 0; i <= LastIndex; i++)
	{
		int32 SwapIndex = FMath::RandRange(i, LastIndex);
		if (i != SwapIndex)
		{
			ArrayToShuffle.Swap(i, SwapIndex);
		}
	}
}

void UGameManager::ModifyArrayStruct(bool bVisitedStatus, int ArrayIndex)
{
	if(MapNodes.IsValidIndex(ArrayIndex))
	{
		MapNodes[ArrayIndex].bVisited = bVisitedStatus;
	}
}

bool UGameManager::isEncounterComplete(FName EncounterName) const
{
	if (CompletedEncounters.Contains(EncounterName))
	{
		return true;
	}
	return false;
}

void UGameManager::MarkEncounterComplete(FName EncounterName)
{
	if (!CompletedEncounters.Contains(EncounterName))
	{
		CompletedEncounters.Add(EncounterName);
	}
}

void UGameManager::GenerateGrid()
{
	Grid.Empty();
	Grid.SetNum(floors * maxNodesPerFloor);

	for (int32 Floor = 0; Floor < floors; ++Floor)
	{
		for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex)
		{
			int32 FlatIndex = Floor * maxNodesPerFloor + NodeIndex;

			FMapNodeData Node;
			Node.RoomType = EMapRoomCPP::Empty;
			Node.bVisited = false;

			Grid[FlatIndex] = Node;
		}
	}
}

void UGameManager::CreateMap()
{
	// get two different starting nodes on the first floor
	int startNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1);
	int startNodeIndex2 = startNodeIndex;
	while (startNodeIndex2 == startNodeIndex)
	{
		startNodeIndex2 = FMath::RandRange(0, maxNodesPerFloor - 1);
	}

	for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex)
	{
		int32 FlatIndex = 0 * maxNodesPerFloor + NodeIndex;
		if (NodeIndex == startNodeIndex || NodeIndex == startNodeIndex2)
		{
			Grid[FlatIndex].RoomType = EMapRoomCPP::Combat; // or any starting room type
		}
		else
		{
			Grid[FlatIndex].RoomType = EMapRoomCPP::Empty;
		}
	}
}

FMapNodeData UGameManager::GetNode(int32 Floor, int32 NodeIndex)
{
	int32 Index = GetNodeIndex(Floor, NodeIndex);
	if (Grid.IsValidIndex(Index))
	{
		return Grid[Index];
	}
	return FMapNodeData(); // return default if out of range
}

void UGameManager::SetNode(int32 Floor, int32 NodeIndex, FMapNodeData& NewData)
{
	int32 Index = GetNodeIndex(Floor, NodeIndex);
	if (Grid.IsValidIndex(Index))
	{
		Grid[Index] = NewData;
	}
}

int32 UGameManager::GetNodeIndex(int32 Floor, int32 NodeIndex)
{
	return Floor * maxNodesPerFloor + NodeIndex;
}
