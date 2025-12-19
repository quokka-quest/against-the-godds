// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"

void UGameManager::GenerateGrid()
{
	if (!Grid.IsEmpty())
	{
		return;
	}
	else
	{
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
}

void UGameManager::CreateMap()
{
	if (bMapGenerated == false)
	{
		ChooseStartingNodes();
		OnCurrentNodeChanged.Broadcast(StartingNodes);

		for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // 
		{
			int32 FlatIndex = GetNodeIndex(0, NodeIndex); // Floor (0) * maxNodesPerFloor (7) + NodeIndex ( 0 - 6 );
			if (StartingNodes.Contains(NodeIndex))
			{
				Grid[FlatIndex].RoomType = Selected; // set starting nodes to combat rooms
			}
			else
			{
				Grid[FlatIndex].RoomType = DefaultRoomType; // set non-starting nodes on floor 0 to empty
			}
		}

		CreateMapPaths();

		for (int32 Floor = 0; Floor < floors; ++Floor) // for each floor
		{
			if (Floor == 9)
			{
				for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // for each node in floor
				{
					if (Grid[NodeIndex + Floor * maxNodesPerFloor].RoomType == Selected)
					{
						Grid[NodeIndex + Floor * maxNodesPerFloor].RoomType = EMapRoomCPP::Shop; // set all nodes on floor 9 to rest rooms
						RestCount -= 1;
					}

				}
			}

			if (Floor == floors - 1)
			{
				for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // for each node in floor
				{
					if (Grid[NodeIndex + Floor * maxNodesPerFloor].RoomType == Selected)
					{
						Grid[NodeIndex + Floor * maxNodesPerFloor].RoomType = EMapRoomCPP::Shop; // set all nodes on final floor to rest rooms
						RestCount -= 1;
					}

				}
			}

			if (Floor == 0)
			{
				for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // for each node in floor
				{
					int32 FlatIndex = GetNodeIndex(Floor, NodeIndex);
					if (Grid[FlatIndex].RoomType == Selected)
					{
						Grid[FlatIndex].RoomType = EMapRoomCPP::Combat;
						CombatCount -= 1;
					}
				}
			}

			if (Floor != 9 && Floor != floors - 1)
			{
				for (int32 NodeIndex = 0; NodeIndex < maxNodesPerFloor; ++NodeIndex) // for each node in floor
				{
					int32 FlatIndex = GetNodeIndex(Floor, NodeIndex);
					bool bPrevWasShop = false;
					
					if (Grid[FlatIndex].RoomType == Selected)
					{
						int32 RemainingSelectableNodes = 0;
						for (int32 i = FlatIndex; i < Grid.Num(); ++i)
						{
							if (Grid[i].RoomType == Selected)
							{
								RemainingSelectableNodes++;
							}
						}

						if (RemainingSelectableNodes <= 0)
						{
							Grid[FlatIndex].RoomType = EMapRoomCPP::Combat;
							CombatCount = FMath::Max(CombatCount - 1, 0);
							continue;
						}

						struct FWeightedRoom
						{
							EMapRoomCPP Type;
							float Weight;
						};

						TArray<FWeightedRoom> WeightedTypes;
						float TotalWeight = 0.f;

						bool bParentIsRest = false;

						for (int32 ParentIndex : Grid[FlatIndex].IncomingNodes)
						{
							if (Grid[ParentIndex].RoomType == EMapRoomCPP::Shop)
							{
								bParentIsRest = true;
								break;
							}
						}

						if (RestCount > 0 && Floor >= 5 && !bParentIsRest)
						{
							float Weight = (float)RestCount / (float)RemainingSelectableNodes;
							WeightedTypes.Add({ EMapRoomCPP::Shop, Weight });
							TotalWeight += Weight;
						}

						if (CombatCount > 0)
						{
							float Weight = (float)CombatCount / (float)RemainingSelectableNodes;
							WeightedTypes.Add({ EMapRoomCPP::Combat, Weight });
							TotalWeight += Weight;
						}

						if (NonCombatCount > 0)
						{
							float Weight = (float)NonCombatCount / (float)RemainingSelectableNodes;
							WeightedTypes.Add({ EMapRoomCPP::Non_Combat, Weight });
							TotalWeight += Weight;
						}

						if (WeightedTypes.Num() == 0 || TotalWeight <= 0.f)
						{
							Grid[FlatIndex].RoomType = EMapRoomCPP::Combat;
							CombatCount = FMath::Max(CombatCount - 1, 0);
							continue;
						}

						float Roll = FMath::FRandRange(0.f, TotalWeight);
						float Running = 0.f;

						EMapRoomCPP ChosenType = EMapRoomCPP::Combat;

						for (const FWeightedRoom& Entry : WeightedTypes)
						{
							Running += Entry.Weight;
							if (Roll <= Running)
							{
								ChosenType = Entry.Type;
								break;
							}
						}
						Grid[FlatIndex].RoomType = ChosenType;

						switch (ChosenType)
						{
						case EMapRoomCPP::Shop:
							RestCount--;
							break;

						case EMapRoomCPP::Combat:
							CombatCount--;
							break;

						case EMapRoomCPP::Non_Combat:
							NonCombatCount--;
							break;

						default:
							break;
						}
					}
				}
			}

		}
		bMapGenerated = true;
	}


	
}

void UGameManager::CreateMapPaths()
{
	TArray<int32> BranchStack;
	TSet<int32> Processed;

	for (int32 StartColumn : StartingNodes) // for each starting node, make a start column variable
	{
		int32 StartFlat = GetNodeIndex(0, StartColumn);

		if (Grid.IsValidIndex(StartFlat))
		{
			Grid[StartFlat].RoomType = Selected;
			BranchStack.Add(StartFlat);
		}
	}

	float BranchProbability = 0.35f;

	while (BranchStack.Num() > 0)
	{
		int32 CurrentFlat = BranchStack.Last();
		BranchStack.Pop();

		if (Processed.Contains(CurrentFlat))
		{
			continue;
		}
		Processed.Add(CurrentFlat);

		int32 Floor = CurrentFlat / maxNodesPerFloor;
		int32 CurrentColumn = CurrentFlat % maxNodesPerFloor;

		if (Floor >= floors - 1)
		{
			continue;
		}

		TArray<int32> NextFlats = ChooseNextNodes(Floor, CurrentColumn);

		for (int32 i = 0; i < NextFlats.Num(); ++i)
		{
			int32 SwapIndex = FMath::RandRange(i, NextFlats.Num() - 1);
			if (i != SwapIndex)
			{
				NextFlats.Swap(i, SwapIndex);
			}
		}

		bool PickedABranch = false;
		bool ShouldCreateBranch = false;

		for (int32 NextFlat : NextFlats)
		{
			if (!Grid.IsValidIndex(NextFlat))
			{
				continue;
			}

			if (!PickedABranch)
			{
				ShouldCreateBranch = true;
			}

			else
			{
				ShouldCreateBranch = FMath::FRand() < BranchProbability;
			}

			if (!ShouldCreateBranch)
			{
				continue;
			}

			Grid[NextFlat].RoomType = EMapRoomCPP::Selected;
			Grid[CurrentFlat].ConnectedNodes.AddUnique(NextFlat);
			Grid[NextFlat].IncomingNodes.AddUnique(CurrentFlat);

			PickedABranch = true;

			if (!Processed.Contains(NextFlat))
			{
				BranchStack.Add(NextFlat);
			}
		}
	}
}

void UGameManager::ChooseStartingNodes()
{
	int startNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1); // choose a random start node
	StartingNodes.Add(startNodeIndex);

	int startNodeIndex2 = startNodeIndex;
	while (startNodeIndex2 == startNodeIndex)
	{
		startNodeIndex2 = FMath::RandRange(0, maxNodesPerFloor - 1); // choose a different random start node
	}

	StartingNodes.Add(startNodeIndex2);

	int extraNodes = 3;
	for (int i = 0; i < extraNodes; ++i)
	{
		int ExtraNodeIndex = FMath::RandRange(0, maxNodesPerFloor - 1);

		if (!StartingNodes.Contains(ExtraNodeIndex))
		{
			StartingNodes.Add(ExtraNodeIndex); // add up to four extra starting nodes (if not already present)
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

	const int32 NextFloor = Floor + 1; // increment floor
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

	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		int32 SwapIndex = FMath::RandRange(i, Candidates.Num() - 1);
		if (i != SwapIndex)
		{
			Candidates.Swap(i, SwapIndex);
		}
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

bool UGameManager::TryMoveToNode(int32 TargetNodeIndex)
{
	// If no node selected yet, allow any starting node
	if (CurrentNodeIndex == -1)
	{
		if (StartingNodes.Contains(TargetNodeIndex))
		{
			CurrentNodeIndex = TargetNodeIndex;
			Grid[CurrentNodeIndex].bVisited = true;
			OnCurrentNodeChanged.Broadcast(Grid[CurrentNodeIndex].ConnectedNodes);
			return true;
		}
		return false;
	}

	// Normal movement logic
	if (Grid[CurrentNodeIndex].ConnectedNodes.Contains(TargetNodeIndex))
	{
		CurrentNodeIndex = TargetNodeIndex;
		Grid[CurrentNodeIndex].bVisited = true;
		OnCurrentNodeChanged.Broadcast(Grid[CurrentNodeIndex].ConnectedNodes);
		return true;
	}
	return false;
}

void UGameManager::BroadcastCurrentNodeConnections()
{
	if (CurrentNodeIndex >= 0 && Grid.IsValidIndex(CurrentNodeIndex))
	{
		OnCurrentNodeChanged.Broadcast(Grid[CurrentNodeIndex].ConnectedNodes);
	}
}

bool UGameManager::isEncounterCompleted(FName EncounterName)
{
	if (!CompletedEncounters.Contains(EncounterName))
	{
		return false;
	}

	return true;
}



