// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class GridDataUIChanger : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	TSharedPtr<IPropertyHandle> ColumnsHandle;
	TSharedPtr<IPropertyHandle> RowsHandle;
	TSharedPtr<IPropertyHandle> CellSizeHandle;
	TSharedPtr<IPropertyHandle> OriginCellCoordHandle;
	TSharedPtr<IPropertyHandle> AllCellsArrayHandle;
};
