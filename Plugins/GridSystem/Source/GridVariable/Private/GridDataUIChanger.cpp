// Fill out your copyright notice in the Description page of Project Settings.


#include "GridDataUIChanger.h"
#include "GridData.h"
#include "GridDataWidgetBuilder.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"

TSharedRef<IPropertyTypeCustomization> GridDataUIChanger::MakeInstance()
{
	return MakeShareable(new GridDataUIChanger());
}

void GridDataUIChanger::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow.NameContent()[StructPropertyHandle->CreatePropertyNameWidget()];
}

void GridDataUIChanger::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	ColumnsHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGridData, Columns));
	RowsHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGridData, Rows));
	CellSizeHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGridData, CellSize));
	OriginCellCoordHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGridData, OriginCellGridCoord));
	AllCellsArrayHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGridData, AllCellsArray));

	// Enable the display of these properties in the details panel (disabled by default)
	ChildBuilder.AddProperty(ColumnsHandle.ToSharedRef());
	ChildBuilder.AddProperty(RowsHandle.ToSharedRef());
	ChildBuilder.AddProperty(OriginCellCoordHandle.ToSharedRef());
	ChildBuilder.AddProperty(CellSizeHandle.ToSharedRef());

	// Custom grid UI widget
	ChildBuilder.AddCustomRow(FText::FromString("Grid"))
	[
		SNew(SGridDataWidgetBuilder)
		.StructHandle(StructPropertyHandle)
		.ColumnsHandle(ColumnsHandle)
		.RowsHandle(RowsHandle)
		.OriginCellCoordHandle(OriginCellCoordHandle)
		.CellSizeHandle(CellSizeHandle)
		.AllCellsArrayHandle(AllCellsArrayHandle)
	];
}