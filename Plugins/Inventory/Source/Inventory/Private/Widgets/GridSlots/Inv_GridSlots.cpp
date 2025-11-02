// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridSlots/Inv_GridSlots.h"

#include "Components/Image.h"

UInv_ItemPopUp* UInv_GridSlots::GetItemPopup() const
{
	return ItemPopUp.Get();
}

void UInv_GridSlots::SetItemPopUp(UInv_ItemPopUp* Popup)
{
	ItemPopUp=Popup;
	ItemPopUp->SetGridIndex(GetIndex());
	ItemPopUp->OnNativeDestruct.AddUObject(this, &ThisClass::onItemPopUpDestruct);
}

void UInv_GridSlots::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseEnter(MyGeometry, MouseEvent);
	GridSlotHovered.Broadcast(TileIndex, MouseEvent);
}

void UInv_GridSlots::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseLeave(MouseEvent);
	GridSlotUnhovered.Broadcast(TileIndex, MouseEvent);
}

FReply UInv_GridSlots::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	GridSlotClicked.Broadcast(TileIndex, MouseEvent);
	return FReply::Handled();
}

void UInv_GridSlots::SetOccupiedTexture()
{
	GridSlotState = EInv_GridSlotState::Occupied;
	Image_GridSlot->SetBrush(Brush_Occupied);
}

void UInv_GridSlots::SetUnoccupiedTexture()
{
	GridSlotState = EInv_GridSlotState::Unoccupied;
	Image_GridSlot->SetBrush(Brush_Unoccupied);
}

void UInv_GridSlots::SetSelectedTexture()
{
	GridSlotState = EInv_GridSlotState::Selected;
	Image_GridSlot->SetBrush(Brush_Selected);
}

void UInv_GridSlots::SetGreyedOutTexture()
{
	GridSlotState = EInv_GridSlotState::GreyedOut;
	Image_GridSlot->SetBrush(Brush_Greyedout);
}

void UInv_GridSlots::SetInventoryItem(UInv_InventoryItem* Item)
{
	InventoryItem = Item;
}

void UInv_GridSlots::onItemPopUpDestruct(UUserWidget* Menu)
{
	ItemPopUp.Reset();
}
