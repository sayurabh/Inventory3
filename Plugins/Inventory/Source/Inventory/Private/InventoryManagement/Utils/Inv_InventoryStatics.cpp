// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Utils/Inv_InventoryStatics.h"

#include "InventoryManagement/Components/Inv_InventoryComponent1.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Types/Inv_InvTypes.h"


UInv_InventoryComponent1* UInv_InventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return nullptr;
	UInv_InventoryComponent1* InventoryComponent = PlayerController->FindComponentByClass<UInv_InventoryComponent1>();
	return InventoryComponent;
}


EInv_ItemCategory UInv_InventoryStatics::GetItemCategoryFromItemComp(UInv_ItemComponent* ItemComp)
{
	if (!IsValid(ItemComp)) return EInv_ItemCategory::None;
	return ItemComp->GetItemManifest().GetItemCategory();
}