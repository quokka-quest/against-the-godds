// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SGridDataWidgetBuilder : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGridDataWidgetBuilder) {}
	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, StructHandle)
	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, ColumnsHandle)
	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, RowsHandle)
	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, OriginCellCoordHandle)
	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, CellSizeHandle)
	SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, AllCellsArrayHandle)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> ColumnsHandle;
	TSharedPtr<IPropertyHandle> RowsHandle;
	TSharedPtr<IPropertyHandle> OriginCellCoordHandle;
	TSharedPtr<IPropertyHandle> CellSizeHandle;
	TSharedPtr<IPropertyHandle> AllCellsHandle;
	
	TSharedPtr<SVerticalBox> GridContainer;

	TSharedRef<SWidget> BuildGrid();
	void RebuildGrid();
};
