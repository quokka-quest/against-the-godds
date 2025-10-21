// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"

TArray<EMapRoomType> UGameManager::GenerateMap()
{
	if(AllRooms.Num() == 0)
	{
		// Populate AllRooms with the required number of each room type
		for(int i = 0; i < CombatCount; i++)
		{
			AllRooms.Add(EMapRoomType::Combat);
		}
		for(int i = 0; i < NonCombatCount; i++)
		{
			AllRooms.Add(EMapRoomType::Non_Combat);
		}
		for(int i = 0; i < RestCount; i++)
		{
			AllRooms.Add(EMapRoomType::Rest);
		}

		// Shuffle the rooms to randomize their order
		ShuffleArray(AllRooms);

		// Add the guaranteed Rest and Boss rooms at the end
		AllRooms.Add(EMapRoomType::Rest);
		AllRooms.Add(EMapRoomType::Boss);

		for (int i = 0; i < AllRooms.Num(); i++)
		{
			MapDataStruct.RoomType = AllRooms[i];
			MapNodes.Add(MapDataStruct);
		}

		// Set the first room as visited
		MapNodes[0].bVisited = true;
		return AllRooms;
	}
	else
	{
		return AllRooms;
	}
}

void UGameManager::ShuffleArray(TArray<EMapRoomType>& ArrayToShuffle)
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
