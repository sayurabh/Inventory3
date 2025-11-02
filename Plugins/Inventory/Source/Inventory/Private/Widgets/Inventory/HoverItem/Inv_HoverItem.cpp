// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/Inv_InventoryItem.h"

void UInv_HoverItem::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}




FGameplayTag UInv_HoverItem::GetItemType() const
{
	if (InventoryItem.IsValid())
	{
		return InventoryItem->GetItemManifest().GetItemType();
	}
	return FGameplayTag();
}

void UInv_HoverItem::SetIsStackable(bool bStacks)
{
	bIsStackable = bStacks;
	if (!bStacks)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UInv_InventoryItem* UInv_HoverItem::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UInv_HoverItem::SetInventoryItem(UInv_InventoryItem* Item)
{
	InventoryItem=Item;
}

void UInv_HoverItem::UpdateStackCount(const int32 count) 
{
	StackCount=count;
	if(count>0)
	{
		//Text_StackCount->SetText(FText::AsNumber(count));
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
		Text_StackCount->SetText(FText::AsNumber(count));
			
	}else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}


