// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory1.h"

#include "Components/Button.h"
#include "Inventory.h"

#include "Components/WidgetSwitcher.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

void UInv_SpatialInventory1::NativeConstruct()
{
	Super::NativeConstruct();
	Button_Equippables->OnClicked.AddDynamic(this,&ThisClass::ShowEquippables);
	Button_Consummables->OnClicked.AddDynamic(this,&ThisClass::ShowConsummables);
	Button_Craftables->OnClicked.AddDynamic(this,&ThisClass::ShowCraftables);
	Grid_Equippables->SetOwningCanvas(CanvasPanel);
	Grid_Consummables->SetOwningCanvas(CanvasPanel);
	Grid_Craftables->SetOwningCanvas(CanvasPanel);
	ShowEquippables();
}

FReply UInv_SpatialInventory1::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	ActiveGrid->DropItem();
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FInv_SlotAvailabilityResult UInv_SpatialInventory1::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
  switch (UInv_InventoryStatics::GetItemCategoryFromItemComp(ItemComponent))
  {
  	case EInv_ItemCategory::Equippable:
  	return Grid_Equippables->HasRoomForItem(ItemComponent);
  	case EInv_ItemCategory::Consumable:
  	return Grid_Consummables->HasRoomForItem(ItemComponent);
  	case EInv_ItemCategory::Craftable:
  	return Grid_Craftables->HasRoomForItem(ItemComponent);
  	default:
  	UE_LOG(MyLogCat, Error, TEXT("ItemComponent doesn't have a valid Item Category."));
  	return FInv_SlotAvailabilityResult();
  }
}

void UInv_SpatialInventory1::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables,Button_Equippables);
}

void UInv_SpatialInventory1::ShowConsummables()
{
	SetActiveGrid(Grid_Consummables,Button_Consummables);
}

void UInv_SpatialInventory1::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables,Button_Craftables);
}

void UInv_SpatialInventory1::DisableButton(UButton* button)
{
	Button_Equippables->SetIsEnabled(true);
	Button_Consummables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	button->SetIsEnabled(false);
}


void UInv_SpatialInventory1::SetActiveGrid(UInv_InventoryGrid* Grid, UButton* button)
{
	if (ActiveGrid.IsValid()) ActiveGrid->HideCursor();
	ActiveGrid = Grid;
	if (ActiveGrid.IsValid()) ActiveGrid->ShowCursor();
	ActiveGrid=Grid;
	DisableButton(button);
	Switcher->SetActiveWidget(Grid);
}
