// Fill out your copyright notice in the Description page of Project Settings.


#include "GridDataWidgetBuilder.h"
#include "GridData.h"
#include "Widgets/Input/SButton.h"

void SGridDataWidgetBuilder::Construct(const FArguments& InArgs)
{
	StructHandle = InArgs._StructHandle;
	ColumnsHandle = InArgs._ColumnsHandle;
	RowsHandle = InArgs._RowsHandle;
	OriginCellCoordHandle = InArgs._OriginCellCoordHandle;
	CellSizeHandle = InArgs._CellSizeHandle;
	AllCellsHandle = InArgs._AllCellsArrayHandle;

	ChildSlot[BuildGrid()];

	// sets the callback function for when the variables have their value changed in the details panel
	if (ColumnsHandle.IsValid())
	{
		ColumnsHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGridDataWidgetBuilder::RebuildGrid));
	}
	if (RowsHandle.IsValid())
	{
		RowsHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGridDataWidgetBuilder::RebuildGrid));
	}
	if (OriginCellCoordHandle.IsValid())
	{
		OriginCellCoordHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGridDataWidgetBuilder::RebuildGrid));

		TSharedPtr<IPropertyHandle> XHandle = OriginCellCoordHandle->GetChildHandle(TEXT("X"));
		TSharedPtr<IPropertyHandle> YHandle = OriginCellCoordHandle->GetChildHandle(TEXT("Y"));

		if (XHandle.IsValid())
		{
			XHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGridDataWidgetBuilder::RebuildGrid));
		}
		if (YHandle.IsValid())
		{
			YHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGridDataWidgetBuilder::RebuildGrid));
		}
	}
	if (CellSizeHandle.IsValid())
	{
		CellSizeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGridDataWidgetBuilder::RebuildGrid));
	}
}

TSharedRef<SWidget> SGridDataWidgetBuilder::BuildGrid()
{
	// return if any of the handles are invalid
	if (!ColumnsHandle.IsValid()) return SNullWidget::NullWidget;
	if (!RowsHandle.IsValid()) return SNullWidget::NullWidget;
	if (!AllCellsHandle.IsValid()) return SNullWidget::NullWidget;
	if (!CellSizeHandle.IsValid()) return SNullWidget::NullWidget;
	if (!StructHandle.IsValid()) return SNullWidget::NullWidget;

	// get the column and row count
	int32 Columns, Rows;
	if (ColumnsHandle->GetValue(Columns) != FPropertyAccess::Success) return SNullWidget::NullWidget;
	if (RowsHandle->GetValue(Rows) != FPropertyAccess::Success) return SNullWidget::NullWidget;

	// get the cell size
	float CellSize;
	if (CellSizeHandle->GetValue(CellSize) != FPropertyAccess::Success) return SNullWidget::NullWidget;

	// get the grid cells array (only need the size so it hasn't been cast to the boolean array type)
	TArray<void*> RawCellArrayData;
	AllCellsHandle->AccessRawData(RawCellArrayData);

	TArray<void*> RawStructData;
	StructHandle->AccessRawData(RawStructData);
	if (RawStructData.Num() == 0) return SNullWidget::NullWidget;
	FGridData* GridData = reinterpret_cast<FGridData*>(RawStructData[0]);
	
	// make the grid widget and set the fill size to 0 to prevent automatic fixed sizing
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel).FillColumn(0 ,0.f).FillRow(0,0.f);

	// get the pixel width and height needed to display the full grid
	float TotalWidth = Columns * CellSize;
	float TotalHeight = Rows * CellSize;

	GridData->CachedColumns = Columns;
	GridData->CachedRows = Rows;
	GridData->CachedOriginCellGridCoord = GridData->OriginCellGridCoord;

	// fill the grid with grid cells (buttons that are inside a size box)
	for (int32 Y = 0; Y < Rows; ++Y)
	{
		for (int32 X = 0; X < Columns; ++X)
		{
			int32 Index = Y * Columns + X;
			bool Selected = false;
			bool IsOrigin = false;

			// if the grid has contents then check if the current cell is 'selected'
			if (GridData->AllCellsArray.Num() > 0)
			{
				// check if the cell has been selected
				if (GridData->AllCellsArray.IsValidIndex(Index)) { Selected = GridData->AllCellsArray[Index]; }
				// check if this cell is the origin
				IsOrigin = GridData->IsCellTheOrigin(Index);
			}

			// creates a grid slot and sets the alignments to centre (needed since it defaults to fill)
			Grid->AddSlot(X, Y).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				// makes a size box and sets its width and height to the correct cell size
				SNew(SBox).WidthOverride(CellSize).HeightOverride(CellSize)
				[
					// makes a button and colours it based on if the cell is selected or not. Also assigns a function to the button
					SNew(SButton)
					.ButtonColorAndOpacity(IsOrigin ? FLinearColor::Red : Selected ? FLinearColor::Green : FLinearColor::Gray)
					.OnClicked_Lambda([this, Index]()
					{
						// the function executed by the button when clicked
						TArray<void*> Data;
						StructHandle->AccessRawData(Data);
						if (Data.Num() > 0)
						{
							FGridData* GridData = reinterpret_cast<FGridData*>(Data[0]);
							if (!GridData->AllCellsArray.IsEmpty() && GridData->AllCellsArray.IsValidIndex(Index))
							{
								// check if the clicked cell is the origin
								if (GridData->IsCellTheOrigin(Index))
								{
									// if the origin cell is clicked then this makes sure it doesn't get deselected
									GridData->AllCellsArray[Index] = false;
								}
								
								// inverts the bool array's value for this button's index
								GridData->AllCellsArray[Index] = !(GridData->AllCellsArray[Index]);

								// rebuild the grid to update the colour in the details panel
								AllCellsHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
								RebuildGrid();
							}
						}
						return FReply::Handled();
					})
				]
			];
		}
	}

	// make scroll bars for the horizontal and vertical boxes
	TSharedRef<SScrollBar> HorizontalScroll =
		SNew(SScrollBar).Orientation(Orient_Horizontal).Thickness(FVector2D(8.f, 8.f));
	
	TSharedRef<SScrollBar> VerticalScroll =
		SNew(SScrollBar).Orientation(Orient_Vertical).Thickness(FVector2D(8.f, 8.f));

	// returns the widget to be displayed
	// the first layer is a size box to give the display a fixed size
	// the second layer is an overlay (this makes both the scroll bars visible constantly)
	// the next layer is a vertical scroll box
	// the next layer is a horizontal scroll box
	// the final layer is the grid itself
	TSharedRef<SWidget> ScrollableGrid =
		SNew(SBox).WidthOverride(500.0f).HeightOverride(500.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(VerticalScroller, SScrollBox).Orientation(Orient_Vertical).ExternalScrollbar(VerticalScroll)
				+ SScrollBox::Slot()
				[
					SAssignNew(HorizontalScroller, SScrollBox).Orientation(Orient_Horizontal).ExternalScrollbar(HorizontalScroll)
					+ SScrollBox::Slot()
					[
						Grid
					]
				]
			]
			+SOverlay::Slot().VAlign(VAlign_Bottom)
			[
				HorizontalScroll
			]
			+SOverlay::Slot().HAlign(HAlign_Right)
			[
				VerticalScroll
			]
		];

	// timer for setting the offsets to the saved values
	TWeakPtr<SScrollBox> WeakV = VerticalScroller;
	TWeakPtr<SScrollBox> WeakH = HorizontalScroller;
	float ToRestoreV = VerticalPos;
	float ToRestoreH = HorizontalPos;

	RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateLambda(
		[WeakV, WeakH, ToRestoreV, ToRestoreH](double, float)
		-> EActiveTimerReturnType
		{
			if (TSharedPtr<SScrollBox> V = WeakV.Pin())
			{
				V->SetScrollOffset(ToRestoreV);
			}
			if (TSharedPtr<SScrollBox> H = WeakH.Pin())
			{
				H->SetScrollOffset(ToRestoreH);
			}
			return EActiveTimerReturnType::Stop; // run once
		}));

	return ScrollableGrid;
}

// This function is called when the Values of Height and Width are changed in the details panel
void SGridDataWidgetBuilder::RebuildGrid()
{
	// exit if any of the handles are invalid
	if (!StructHandle.IsValid()) return;
	if (!ColumnsHandle.IsValid()) return;
	if (!RowsHandle.IsValid()) return;
	if (!OriginCellCoordHandle.IsValid()) return;
	if (!AllCellsHandle.IsValid()) return;

	// preserve the scroll bars positions
	VerticalPos = VerticalScroller->GetScrollOffset();
	HorizontalPos = HorizontalScroller->GetScrollOffset();

	// get the width and height and exit if the returned values are invalid
	int32 Columns;
	int32 Rows;
	if (ColumnsHandle->GetValue(Columns) != FPropertyAccess::Success) return;
	if (RowsHandle->GetValue(Rows) != FPropertyAccess::Success) return;

	// Gets the origin cell's grid coordinate value
	FIntVector2 OriginCellGridCoord;
	TSharedPtr<IPropertyHandle> XHandle = OriginCellCoordHandle->GetChildHandle(TEXT("X"));
	TSharedPtr<IPropertyHandle> YHandle = OriginCellCoordHandle->GetChildHandle(TEXT("Y"));
	int32 XCoord = 0;
	int32 YCoord = 0;
	if (XHandle.IsValid()) XHandle->GetValue(XCoord);
	if (YHandle.IsValid()) YHandle->GetValue(YCoord);
	OriginCellGridCoord = FIntVector2(XCoord, YCoord);

	// get the FGridData struct data and return if it's invalid
	TArray<void*> RawStructData;
	StructHandle->AccessRawData(RawStructData);
	if (RawStructData.Num() == 0) return;

	FGridData* GridData = reinterpret_cast<FGridData*>(RawStructData[0]);
	if (!GridData) return;

	// get the current cell array and the old grid size and origin cell coord
	TArray<bool> OldCells = GridData->AllCellsArray;
	int32 OldColumns = GridData->CachedColumns;
	int32 OldRows = GridData->CachedRows;
	FIntVector2 OldOriginCoord = GridData->CachedOriginCellGridCoord;

	// Rebuild the cell array so it is empty and of the new size
	GridData->AllCellsArray.Empty();
	GridData->AllCellsArray.SetNum(Columns * Rows);

	// Get the old and new index for a given cartesian cell coordinate
	// match those indices values (keeps the grid selected cells in the correct visual position when re-sizing the grid)
	for (int32 Y = 0; Y < OldRows && Y < Rows; ++Y)
	{
		for (int32 X = 0; X < OldColumns && X < Columns; ++X)
		{
			int32 OldIndex = Y * OldColumns + X;
			int32 NewIndex = Y * Columns + X;
			if (OldCells.IsValidIndex(OldIndex) && GridData->AllCellsArray.IsValidIndex(NewIndex))
			{
				GridData->AllCellsArray[NewIndex] = OldCells[OldIndex];

				// if the cell is the old origin coord but the origin coord has been changed then deselect the old coord
				if (FIntVector2(X, Y) == OldOriginCoord && OldOriginCoord !=OriginCellGridCoord)
				{
					GridData->AllCellsArray[NewIndex] = false;
				}
			}
		}
	}

	// Update cached dimensions for next resize
	GridData->CachedColumns = Columns;
	GridData->CachedRows = Rows;
	GridData->CachedOriginCellGridCoord = OriginCellGridCoord;

	AllCellsHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	// Replace the grid in the widget hierarchy
	ChildSlot.DetachWidget();
	ChildSlot.AttachWidget(BuildGrid());
}