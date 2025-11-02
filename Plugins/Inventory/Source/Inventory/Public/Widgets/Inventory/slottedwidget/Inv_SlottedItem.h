// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Inv_SlottedItem.generated.h"

class UTextBlock;
class UInv_InventoryItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlottedItemClicked,int32,GridIndex,const FPointerEvent&,MouseEvent);
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	void setIsStackcable(bool bStackcable){bIsStackable=bStackcable;}
  UImage* GetImageIcon() const {return Image_Icon;}
	void setGridIndex(int32 Index){GridIndex = Index;}
	int32 GetGridIndex() const {return GridIndex;}
	void SetGridDimension(const FIntPoint& Dimensions ) {GridDimensions = Dimensions;}
	FIntPoint GetGridDimension() const {return GridDimensions;}
	void SetInventoryItem(UInv_InventoryItem* Item);
	UInv_InventoryItem* GetInventoryItem() const { return InventoryItem.Get(); }
	void SetImageBrush(const FSlateBrush& Brush) const;
	void  UpdateStackCount(int32 StackCount) ;
	FSlottedItemClicked OnSlottedItemClicked;
private:
UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage>Image_Icon;

	int32 GridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bIsStackable{false};
};
