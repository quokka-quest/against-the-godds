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
	Grid.Empty(); // change to if grid empty, make grid. Else, do nothing
	Grid.SetNum(floors * maxNodesPerFloor);

	for (int32 Floor = 0; Floor < floors; ++Floor) // for each floor
	{
		for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // for each node in floor
		{
			int32 FlatIndex = Floor * maxNodesPerFloor + NodeIndex; 

			FMapNodeData Node;
			Node.RoomType = EMapRoomCPP::Empty; // default to empty
			Node.bVisited = false; // default to not visited

			Grid[FlatIndex] = Node; // add to the grid
		}
	}
}

void UGameManager::CreateMap()
{
	TArray<int> StartingNodes;

	int startNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1); // choose a random start node
	StartingNodes.Add(startNodeIndex);

	int startNodeIndex2 = startNodeIndex;
	while (startNodeIndex2 == startNodeIndex)
	{
		startNodeIndex2 = FMath::RandRange(0, maxNodesPerFloor - 1); // choose a different random start node
	}

	StartingNodes.Add(startNodeIndex2);

	int extraNodes = 4;
	for (int i = 0; i < extraNodes; ++i)
	{
		int ExtraNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1);

		if (!StartingNodes.Contains(ExtraNodeIndex))
		{
			StartingNodes.Add(ExtraNodeIndex); // add up to four extra starting nodes (if not already present)
		}
	}

	for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // 
	{
		int32 FlatIndex = GetNodeIndex(0, NodeIndex);
		if (StartingNodes.Contains(NodeIndex))
		{
			Grid[FlatIndex].RoomType = EMapRoomCPP::Combat; // set starting nodes to combat rooms
		}
		else
		{
			Grid[FlatIndex].RoomType = EMapRoomCPP::Empty; // set non-starting nodes on floor 0 to empty
		}
	}

	for (int32 StartColumn : StartingNodes)
	{
		int32 CurrentColumn = StartColumn;
		int32 CurrentFlat = GetNodeIndex(0, CurrentColumn);

		if (Grid.IsValidIndex(CurrentFlat))
		{
			Grid[CurrentFlat].RoomType = EMapRoomCPP::Combat;
		}

		for (int32 Floor = 0; Floor < floors - 1; ++Floor)
		{
			TArray<int32> NextFlats = ChooseNextNodes(Floor, CurrentColumn);

			for (int32 NextFlat : NextFlats)
			{
				if (Grid.IsValidIndex(NextFlat))
				{
					Grid[NextFlat].RoomType = EMapRoomCPP::Combat;
					Grid[CurrentFlat].ConnectedNodes.AddUnique(NextFlat);
					Grid[NextFlat].IncomingNodes.AddUnique(CurrentFlat);
				}
			}

			if (NextFlats.Num() > 0)
			{
				CurrentColumn = NextFlats[0] % maxNodesPerFloor;
				CurrentFlat = NextFlats[0];
			}
		}
	}

	for (int32 room = 0; room < Grid.Num(); ++room)
	{
		if (Grid[room].ConnectedNodes.Num() != 0)
		{
			// make a text variable
			FString Log = ("Room " + FString::FromInt(room) + " has connections to:");
			for (int32 connectedRoom : Grid[room].ConnectedNodes)
			{
				Log += " " + FString::FromInt(connectedRoom);
			}
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Log);
		}
	}

}

bool UGameManager::DoesConnectionCross(int32 Floor, int32 ColA, int32 ColB)
{
    for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex)
    {
        int32 FlatIndex = GetNodeIndex(Floor, NodeIndex);
        for (int32 ConnectedFlat : Grid[FlatIndex].ConnectedNodes)
        {
            int32 ExistingColA = NodeIndex;
            int32 ExistingColB = ConnectedFlat % maxNodesPerFloor;

            if ((ColA < ExistingColA && ColB > ExistingColB) ||
                (ColA > ExistingColA && ColB < ExistingColB))
            {
                return true;
            }
        }
    }
    return false;
}
TArray<int32> UGameManager::ChooseNextNodes(int32 Floor, int32 NodeIndex)
{
	TArray<int32> Result;
	if (Floor < 0 || Floor >= floors - 1) return Result;
	if (NodeIndex < 0 || NodeIndex >= maxNodesPerFloor) return Result;

	const int32 NextFloor = Floor + 1;
	const int32 LastCol = maxNodesPerFloor - 1;
	TArray<int32> Candidates;

	if (NodeIndex == 0)
	{
		if (0 <= LastCol) Candidates.Add(0);
		if (1 <= LastCol) Candidates.Add(1);
	}
	else if (NodeIndex == LastCol)
	{
		if (LastCol - 1 >= 0) Candidates.Add(LastCol - 1);
		Candidates.Add(LastCol);
	}
	else
	{
		Candidates.Add(FMath::Clamp(NodeIndex - 1, 0, LastCol));
		Candidates.Add(NodeIndex);
		Candidates.Add(FMath::Clamp(NodeIndex + 1, 0, LastCol));
	}

	for (int32 ChosenCol : Candidates)
	{
		if (!DoesConnectionCross(Floor, NodeIndex, ChosenCol))
		{
			int32 ChosenFlat = GetNodeIndex(NextFloor, ChosenCol);
			Result.Add(ChosenFlat);
			if (Result.Num() == 2) break;
		}
	}

	return Result;
}



FMapNodeData UGameManager::GetNode(int32 Floor, int32 NodeIndex)
{
	int32 Index = GetNodeIndex(Floor, NodeIndex);
	if (Grid.IsValidIndex(Index))
	{
		return Grid[Index];
	}
	return FMapNodeData();
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

void UGameManager::setCurrentNodePosition(int32 node, float x, float y)
{
	Grid[node].positionX = x;
	Grid[node].positionY = y;
}
