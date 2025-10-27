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
