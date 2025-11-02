// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/Inv_HUDWidget.h"
#include "InventoryManagement/Components/Inv_InventoryComponent1.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Widgets/HUD/Inv_InfoMessages.h"

void UInv_HUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UInv_InventoryComponent1* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->NoRoomInInventory.AddDynamic(this, &UInv_HUDWidget::OnNoRoom);
	}
	
}

void UInv_HUDWidget::OnNoRoom()
{
	if (!IsValid(InfoMessage)) return;
	InfoMessage->SetMessage(FText::FromString("No Room In Inventory."));
}
