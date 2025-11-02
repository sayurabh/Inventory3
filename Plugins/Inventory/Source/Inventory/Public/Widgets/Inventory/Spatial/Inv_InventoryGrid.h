// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Inv_InventoryItem.h"
#include "Types/Inv_InvTypes.h"
#include "Inv_InventoryGrid.generated.h"

class UInv_ItemPopUp;
class UInv_HoverItem;
struct FInv_ImageFragment;
struct FInv_GridFragment;
class UInv_SlottedItem;
class UInv_ItemComponent;
class UInv_InventoryComponent1;
class UCanvasPanel;
class UInv_GridSlots;
struct FGameplayTag;
struct FInv_ItemManifest;
enum class EInv_GridSlotState:uint8;




/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item);
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& Manifest);
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);
	virtual void NativeOnInitialized() override;
	EInv_ItemCategory GetItemCategory() const {return ItemCategory; }
	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);
	void ShowCursor();
	void HideCursor();
	void DropItem();
	void SetOwningCanvas(UCanvasPanel* OwningCanvas);
private:
	//void DropItem();
	UFUNCTION()
	void OnPopUpMenuDrop(int32 index);
	UFUNCTION()
	void OnPopUpMenuConsume(int32 index);
	UFUNCTION()
	void OnPopUpMenuSplit(int32 SplitAmount,int32 index);
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInv_ItemPopUp> ItemPopupClass;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D ItemPopUpOffset;
UPROPERTY()
	
	TObjectPtr<UInv_ItemPopUp> ItemPopUp;
	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;
	void CreateItemPopUp(const int32 GridIndex);
	void FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index);
	void SwapStackCounts(const int32 ClickedStackCount,const int32 HoveredStackCount,const int32 Index);
	bool IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const;
	int32 LastHighlightIndex;
	bool ShouldfillInStacks(const int32 RoomInClickedSlot,const int32 HoveredStackCount) const;
	bool ShouldConsumeHoverItemStacks(const int32 HoverStackCount,const int32 RoomInClickedSlot) const;
void ConsumerHoverItemStacks(const int32 ClickedStackCount,const int32 HoveredStackCount,const int32 Index);
	bool shouldSwapStackCount(const int32 RoomInClickedSlot,const int32 HoveredStackCount,const int32 MaxStackSize) const;
	void SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem,const int32 GridIndex);
	FIntPoint LastHighlightedDimensions;
	void HighLight(const int32 Index,const FIntPoint& Dimensions);
	void UnHighLight(const int32 Index,const FIntPoint& Dimensions);
	bool bLastMouseWithinCanvas;
	bool bMouseWithinCanvas;
	bool CursorExitedCanvas(const FVector2D& BoundaryPos,const FVector2D& BoundarySize,const FVector2D& Location);
	FInv_SpaceQueryResult CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions) ;
	 bool IsRightClick(const FPointerEvent& MouseEvent) const;
	bool IsLeftClick(const FPointerEvent& MouseEvent) const;
	void AddITtemToIndices(const FInv_SlotAvailabilityResult& Result,UInv_InventoryItem* newItem);

	bool MatchesCategory(const UInv_InventoryItem* item) const;
   TWeakObjectPtr<UInv_InventoryComponent1> InventoryComponent;
	void ConstrcutGrid();

	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;

	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem;

	int32 ItemDropIndex{INDEX_NONE};
	FInv_SpaceQueryResult CurrentQueryResult;
	FIntPoint CalculateStartingCoordinate(const FIntPoint& Coordinate,const FIntPoint& Dimensions,EInv_TileQuadrant Quadrant) const;
	UPROPERTY(EditAnywhere, Category="Inventory",BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	EInv_ItemCategory ItemCategory;
	FInv_TileParameters TileParameters;
	FInv_TileParameters LastTileParameters;
	void PutDownOnIndex(const int32 Index);
	void ClearHoverItem();
	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent);
	EInv_TileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;
	FIntPoint CalculateHoveredCoordinates(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;
	UPROPERTY()
	TArray<TObjectPtr<UInv_GridSlots>> GridSlots;
   void OnTileParametersUpdate(const FInv_TileParameters& Parameters);
	void UpdateTileParameters(const FVector2D& CanvasPosition,const FVector2D& MousePosition);
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInv_GridSlots> GridSlotClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
   void ChangeHoverType(const int32 Index,const FIntPoint& Dimensions,EInv_GridSlotState GridSlotState);
	int32 DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountToFill, const UInv_GridSlots* GridSlot) const;
	int32 GetStackAmount(const UInv_GridSlots* GridSlot) const;

UUserWidget* GetVisibleCursorWidget();
	UUserWidget* GetHiddenCursorWidget();
	UPROPERTY()
	TObjectPtr<UUserWidget> VisibleCursorWidget;
	UPROPERTY()
	TObjectPtr<UUserWidget> HiddenCursorWidget;
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UUserWidget> HiddenCursorWidgetClass;
UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UUserWidget> VisibleCursorWidgetClass;
	UPROPERTY(EditAnywhere,Category="Inventory")
	int32 Rows;
     void UpdateGridSlots(UInv_InventoryItem* NewItem,const int32 Index,bool bStackableItem,const int32 StackAmount);
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;
	UPROPERTY(EditAnywhere,Category="Inventory")
	int32 Columns;
	
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const;
	void SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment) const;
	void AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);
	UInv_SlottedItem* CreateSlottedItem(UInv_InventoryItem* Item,
		const bool bStackable,
		const int32 StackAmount,
		const FInv_GridFragment* GridFragment,
		const FInv_ImageFragment* ImageFragment,
		const int32 Index);
	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const;
	UPROPERTY()
	TMap<int32, TObjectPtr<UInv_SlottedItem>> SlottedItems;
	UPROPERTY(EditAnywhere,Category="Inventory")
	float TileSize;	
	bool IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const;
	bool DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const;
	bool IsUpperLeftSlot(const UInv_GridSlots* GridSlot, const UInv_GridSlots* SubGridSlot) const;
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const;
void AssignHoverItem(UInv_InventoryItem* InventoryItem);
void RemoveItemFromGrid(UInv_InventoryItem* InventoryItem,const int32 GridIndex);
	void AssignHoverItem(UInv_InventoryItem* InventoryItem,const int32 GridIndex,const int32 PreviousGridIndex);
	void PickUp(UInv_InventoryItem* ClickedInventoryItem,const int32 GridIndex);
	UFUNCTION()
	void AddStacks(const FInv_SlotAvailabilityResult& Result);
UFUNCTION()
	void OnSlottedItemClickec(int32 GridIndex,const FPointerEvent& MouseEvent );
	bool HasRoomAtIndex(const UInv_GridSlots* GridSlot,
	const FIntPoint& Dimensions,

	const TSet<int32>& CheckedIndices,
	TSet<int32>& OutTentativelyClaimed,
	const FGameplayTag& ItemType,
	const int32 MaxStackSize);
	bool HasValidItem(const UInv_GridSlots* GridSlot) const;
	FIntPoint GetItemDimensions(const FInv_ItemManifest& Manifest) const;
bool CheckSlotConstraints(const UInv_GridSlots* GridSlot,const UInv_GridSlots* SubGridSlot, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
	const FGameplayTag& ItemTypes,const int32 MaxStackSize) const;
};
