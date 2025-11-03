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
	TArray<int> StartingNodes;

	int startNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1);
	StartingNodes.Add(startNodeIndex);

	int startNodeIndex2 = startNodeIndex;
	while (startNodeIndex2 == startNodeIndex)
	{
		startNodeIndex2 = FMath::RandRange(0, maxNodesPerFloor - 1);
	}

	StartingNodes.Add(startNodeIndex2);

	// set four more starting nodes on first floor (can be dupes)
	int extraNodes = 4;
	for (int i = 0; i < extraNodes; ++i)
	{
		int ExtraNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1);

		if (!StartingNodes.Contains(ExtraNodeIndex))
		{
			StartingNodes.Add(ExtraNodeIndex);
		}
	}

	// Mark first floor nodes correctly (StartingNodes contains columns, not flat indices)
	for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex)
	{
		int32 FlatIndex = GetNodeIndex(0, NodeIndex);
		if (StartingNodes.Contains(NodeIndex))
		{
			Grid[FlatIndex].RoomType = EMapRoomCPP::Combat; // or any starting room type
		}
		else
		{
			Grid[FlatIndex].RoomType = EMapRoomCPP::Empty;
		}
	}

	// For each starting node (column), create paths down the floors
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
			// Get possible next nodes (returns array of candidates)
			TArray<int32> NextFlats = ChooseNextNodes(Floor, CurrentColumn);

			for (int32 NextFlat : NextFlats)
			{
				if (Grid.IsValidIndex(NextFlat))
				{
					Grid[NextFlat].RoomType = EMapRoomCPP::Combat;
					// Record the connection
					Grid[CurrentFlat].ConnectedNodes.AddUnique(NextFlat);
					Grid[NextFlat].IncomingNodes.AddUnique(CurrentFlat);
				}
			}

			// For next iteration, you may want to branch further from each NextFlat
			// This requires a queue or recursion for full branching
			// For simplicity, you can continue with the first branch:
			if (NextFlats.Num() > 0)
			{
				CurrentColumn = NextFlats[0] % maxNodesPerFloor;
				CurrentFlat = NextFlats[0];
			}
		}
	}

}

bool UGameManager::DoesConnectionCross(int32 Floor, int32 ColA, int32 ColB)
{
    // For each existing connection on this floor
    for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex)
    {
        int32 FlatIndex = GetNodeIndex(Floor, NodeIndex);
        for (int32 ConnectedFlat : Grid[FlatIndex].ConnectedNodes)
        {
            int32 ExistingColA = NodeIndex;
            int32 ExistingColB = ConnectedFlat % maxNodesPerFloor;

            // Check for crossing
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

	// Filter out candidates that would cross existing connections
	for (int32 ChosenCol : Candidates)
	{
		if (!DoesConnectionCross(Floor, NodeIndex, ChosenCol))
		{
			int32 ChosenFlat = GetNodeIndex(NextFloor, ChosenCol);
			Result.Add(ChosenFlat);
			if (Result.Num() == 2) break; // Only two branches
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
