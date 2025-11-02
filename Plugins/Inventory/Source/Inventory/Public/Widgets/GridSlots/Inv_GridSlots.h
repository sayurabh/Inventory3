// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/ItemPopUp/Inv_ItemPopup.h"
#include "Inv_GridSlots.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, GridIndex, const FPointerEvent&, MouseEvent);
class UImage;
/**
 * 
 */


UENUM(BlueprintType)
enum class EInv_GridSlotState : uint8
{
	Unoccupied,
	Occupied,
	Selected,
	GreyedOut
};
UCLASS()
class INVENTORY_API UInv_GridSlots : public UUserWidget
{
	GENERATED_BODY()
public:
	UInv_ItemPopUp* GetItemPopup() const ;
	void SetItemPopUp(UInv_ItemPopUp* Popup);
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
   virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void SetOccupiedTexture();
	void SetUnoccupiedTexture();
	void SetSelectedTexture();
	void SetGreyedOutTexture();
	void SetTileIndex(const int32 Index){TileIndex = Index;};
	int32 GetTileIndex() const{return TileIndex;};
	FGridSlotEvent GridSlotClicked;
	FGridSlotEvent GridSlotHovered;
	FGridSlotEvent GridSlotUnhovered;
	EInv_GridSlotState GetGridSlotState() const{return GridSlotState;};
    TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
    TWeakObjectPtr<UInv_ItemPopUp> ItemPopUp;
	bool bAvailable{true};
     TWeakObjectPtr<UInv_InventoryItem> GetInventoryItem() const{return InventoryItem;}
	void SetInventoryItem(UInv_InventoryItem* Item);
	int32 GetStackCount() const {return StackCount;}
	void SetStackCount(int32 Count){StackCount = Count;}
	int32 GetIndex() const {return TileIndex;}
	void SetIndex(int32 Index){TileIndex = Index;};
	int32 GetUpperLeftIndex() const{return UpperLeftIndex;}
	void SetUpperLeftIndex(int32 Index){UpperLeftIndex = Index;};
	bool isAvailable() const{return bAvailable;}
	void SetAvailable(bool b){bAvailable = b;}
	
private:
	
	int32 TileIndex{INDEX_NONE};
	int32 StackCount{0};

	int32 UpperLeftIndex{INDEX_NONE};

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_GridSlot;

	UPROPERTY(EditAnywhere,Category="Inventory")
	FSlateBrush Brush_Unoccupied;
	UPROPERTY(EditAnywhere,Category="Inventory")
	FSlateBrush Brush_Occupied;

	UPROPERTY(EditAnywhere,Category="Inventory")
	FSlateBrush Brush_Selected;

	UPROPERTY(EditAnywhere,Category="Inventory")
	FSlateBrush Brush_Greyedout;

	EInv_GridSlotState GridSlotState;

  UFUNCTION()
	void onItemPopUpDestruct(UUserWidget* Menu);
};
