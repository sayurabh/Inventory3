// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Inventory.h"
#include "SNodePanel.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent1.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragment/Inv_FragmentTags.h"
#include "Items/Fragment/Inv_ItemFragment.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Widgets/GridSlots/Inv_GridSlots.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/utils/Inv_WidgetUtils.h"
#include "Widgets/Inventory/slottedwidget/Inv_SlottedItem.h"
#include "Widgets/ItemPopUp/Inv_ItemPopup.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ConstrcutGrid();
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::AddItem);
	InventoryComponent->OnStackChange.AddDynamic(this, &ThisClass::AddStacks);
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!MatchesCategory(Item)) return;

	UE_LOG(LogTemp, Warning, TEXT("InventoryGrid::AddItemgvyg"));
	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);
	AddITtemToIndices(Result, Item);
}

void UInv_InventoryGrid::ShowCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
		GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default,GetVisibleCursorWidget());
	
}

void UInv_InventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default,GetHiddenCursorWidget());
}

void UInv_InventoryGrid::CreateItemPopUp(const int32 GridIndex)
{
	UInv_InventoryItem* RightClickedItem= GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem)) return;
	if (IsValid(GridSlots[GridIndex]->GetItemPopup())) return;;

	ItemPopUp = CreateWidget<UInv_ItemPopUp>(this,ItemPopupClass);
    GridSlots[GridIndex]->SetItemPopUp(ItemPopUp);
	OwningCanvasPanel->AddChild(ItemPopUp);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasSlot->SetPosition(MousePosition - ItemPopUpOffset);
	CanvasSlot->SetSize(ItemPopUp->GetBoxSize());
const int32 SliderMax = GridSlots[GridIndex]->GetStackCount()-1;
	if (RightClickedItem->IsStackable()&&SliderMax>0)
	{
		ItemPopUp->OnSplit.BindDynamic(this,&ThisClass::OnPopUpMenuSplit);
		ItemPopUp->SetSliderParams(SliderMax,FMath::Max(1,GridSlots[GridIndex]->GetStackCount()/2));
	}else
	{
		ItemPopUp->CollapseSplitButton();
	}
	ItemPopUp->OnDrop.BindDynamic(this,&ThisClass::OnPopUpMenuDrop);

   if (RightClickedItem->IsConsumable())
   {
	   ItemPopUp->OnConsume.BindDynamic(this,&ThisClass::OnPopUpMenuConsume);
   }else
   {
	   ItemPopUp->CollapseConsumeButton();
   }
}
void UInv_InventoryGrid::SetOwningCanvas(UCanvasPanel* OwningCanvas)
{
	OwningCanvasPanel = OwningCanvas;
}

void UInv_InventoryGrid::OnPopUpMenuDrop(int32 index)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem)) return;

	PickUp(RightClickedItem, index);
	DropItem();
}
void UInv_InventoryGrid::DropItem()
{
	if (!IsValid(HoverItem)) return;
	if (!IsValid(HoverItem->GetInventoryItem())) return;
  InventoryComponent->Server_DropItem(HoverItem->GetInventoryItem(),HoverItem->GetStackCount());
	// TODO: Tell the server to actually drop the item

	ClearHoverItem();
	ShowCursor();
}

void UInv_InventoryGrid::OnPopUpMenuConsume(int32 index)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem)) return;

	const int32 UpperLeftIndex = GridSlots[index]->GetUpperLeftIndex();
	UInv_GridSlots* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	// TODO: Tell the server we're consuming an item
         InventoryComponent->Server_ConsumeITem(RightClickedItem);
	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(RightClickedItem, index);
	}
}

void UInv_InventoryGrid::OnPopUpMenuSplit(int32 SplitAmount, int32 index)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem)) return;
	if (!RightClickedItem->IsStackable()) return;

	const int32 UpperLeftIndex = GridSlots[index]->GetUpperLeftIndex();
	UInv_GridSlots* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 StackCount = UpperLeftGridSlot->GetStackCount();
	const int32 NewStackCount = StackCount - SplitAmount;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	AssignHoverItem(RightClickedItem, UpperLeftIndex, UpperLeftIndex);
	HoverItem->UpdateStackCount(SplitAmount);
}

void UInv_InventoryGrid::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount,
                                         const int32 Index)
{
	UInv_GridSlots* GridSlot = GridSlots[Index];
	GridSlot->SetStackCount(HoveredStackCount);
	
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(HoveredStackCount);
   HoverItem->UpdateStackCount(ClickedStackCount);
}

bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	const bool bIsSameItem = ClickedInventoryItem==HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable && HoverItem->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

bool UInv_InventoryGrid::ShouldfillInStacks(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const
{
	return RoomInClickedSlot<HoveredStackCount;
}

bool UInv_InventoryGrid::ShouldConsumeHoverItemStacks(const int32 HoverStackCount, const int32 RoomInClickedSlot) const
{
	return RoomInClickedSlot>=HoverStackCount;
}

void UInv_InventoryGrid::ConsumerHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount+AmountToTransfer;

	GridSlots[Index]->SetStackCount(NewClickedStackCount);
	SlottedItems.FindChecked(Index)->UpdateStackCount(NewClickedStackCount);
 ClearHoverItem();
	ShowCursor();
	const FInv_GridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInv_GridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	HighLight(Index, Dimensions);
}

bool UInv_InventoryGrid::shouldSwapStackCount(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
                                              const int32 MaxStackSize) const
{
	return RoomInClickedSlot==0 && HoveredStackCount<MaxStackSize;
}

void UInv_InventoryGrid::SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if (!IsValid(HoverItem)) return;

	UInv_InventoryItem* TempInventoryItem = HoverItem->GetInventoryItem();
	const int32 TempStackCount = HoverItem->GetStackCount();
	const bool bTempStackable = HoverItem->IsStackable();

	AssignHoverItem(ClickedInventoryItem,GridIndex,HoverItem->GetPreviousGridIndex());
		RemoveItemFromGrid(ClickedInventoryItem,GridIndex);
	AddItemAtIndex(TempInventoryItem,ItemDropIndex,bTempStackable,TempStackCount);
	UpdateGridSlots(TempInventoryItem,ItemDropIndex,bTempStackable,TempStackCount);
}


void UInv_InventoryGrid::HighLight(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas) return;
	//UE_LOG(LogTemp, Warning, TEXT("The : %d"), 34);
      UnHighLight(LastHighlightIndex,LastHighlightedDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Columns,[&](UInv_GridSlots* GridSlot)
	{
	//	UE_LOG(LogTemp, Warning, TEXT("T : %d"), 34);
		GridSlot->SetOccupiedTexture();
	});
	LastHighlightedDimensions = Dimensions;
	LastHighlightIndex = Index;
	
}

void UInv_InventoryGrid::UnHighLight(const int32 Index, const FIntPoint& Dimensions)
{
	UInv_InventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Columns,[&](UInv_GridSlots* GridSlot)
	{
		if (GridSlot->isAvailable())
		{
			GridSlot->SetUnoccupiedTexture();
		}else
		{
			GridSlot->SetOccupiedTexture();
		}
	});
}

bool UInv_InventoryGrid::CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundarySize,
                                            const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UInv_WidgetUtils::IsWithinBounds(BoundaryPos, BoundarySize, Location);
	if (!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		// TODO: UnhighlightSlots()
		UnHighLight(LastHighlightIndex,LastHighlightedDimensions);
		return true;
	}
	return false;
}

FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position,
                                                             const FIntPoint& Dimensions) 
{
	FInv_SpaceQueryResult Result;
	if (!IsInGridBounds(UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions)) return Result;
	Result.bHasSpace = true;
	
	// If more than one of the indices is occupied with the same item, we need to see if they all have the same upper left index.
	TSet<int32> OccupiedUpperLeftIndices;
	UInv_InventoryStatics::ForEach2D(GridSlots, UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions, Columns, [&](const UInv_GridSlots* GridSlot)
	{
		if (GridSlot->GetInventoryItem().IsValid())
		{
			OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
			Result.bHasSpace = false;
		}
	});

	// any items in the way?
	// if so, is there only one item in the way? (can we swap?)
	if (OccupiedUpperLeftIndices.Num() == 1) // single item at position - it's valid for swapping/combining
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[Index]->GetInventoryItem();
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}

	return Result;
}

bool UInv_InventoryGrid::IsRightClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInv_InventoryGrid::IsLeftClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable,
                                                        const int32 StackAmount, const FInv_GridFragment* GridFragment,
                                                        const FInv_ImageFragment* ImageFragment, const int32 Index)
{
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetInventoryItem(Item);
	SetSlottedItemImage(SlottedItem, GridFragment, ImageFragment);
	SlottedItem->setGridIndex(Index);

	SlottedItem->setIsStackcable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClickec);

	return SlottedItem;
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem,
                                         const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
	//	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
	if (!GridFragment) return;
	if (bStackableItem)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);

	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlots* GridSlot)
	{
		GridSlot->SetAvailable(false);
		GridSlot->SetUpperLeftIndex(Index);
		GridSlot->SetInventoryItem(NewItem);
		GridSlot->SetOccupiedTexture();
	});
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment,
                                             const FInv_ImageFragment* ImageFragment) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable,
                                        const int32 StackAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("The stackamount : %d"), StackAmount);
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;

	UInv_SlottedItem* SlottedItem =
		CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	UE_LOG(LogTemp, Warning, TEXT("The float valuebhfgy: %d"), Index);
	SlottedItems.Add(Index, SlottedItem);
	// Add the slotted item to the canvas panel.

	// Store the new widget in a container.
}

void UInv_InventoryGrid::AddITtemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		UE_LOG(LogTemp, Warning, TEXT("The float value: %d"), Availability.Index);
		UE_LOG(LogTemp, Warning, TEXT("The float value is lulu: %d"), Availability.AmountTofill);
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountTofill);
		UpdateGridSlots(NewItem, Availability.Index, Result.bStackable, Availability.AmountTofill);
	}
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPosition,
                                                            const FVector2D& MousePosition) const
{
	// Calculate relative position within the current tile
	const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize);
	const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize);

	// Determine which quadrant the mouse is in
	const bool bIsTop = TileLocalY < TileSize / 2.f; // Top if Y is in the upper half
	const bool bIsLeft = TileLocalX < TileSize / 2.f; // Left if X is in the left half

	EInv_TileQuadrant HoveredTileQuadrant{EInv_TileQuadrant::None};
	if (bIsTop && bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::TopLeft;
	else if (bIsTop && !bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::TopRight;
	else if (!bIsTop && bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::BottomLeft;
	else if (!bIsTop && !bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::BottomRight;

	return HoveredTileQuadrant;
}

FIntPoint UInv_InventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPosition,
                                                          const FVector2D& MousePosition) const
{
	return FIntPoint{
		static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X) / TileSize)),
		static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y) / TileSize))
	};
}


void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("The float adc: %d"), 34);
	const FVector2D CanvasPosition = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
  if (CursorExitedCanvas(CanvasPosition,UInv_WidgetUtils::GetWidgetSize(CanvasPanel),MousePosition))
  {
  //	UE_LOG(LogTemp, Warning, TEXT("The float acdc: %d"), 34);
	  return;
  }
	//UE_LOG(LogTemp, Warning, TEXT("The float auc: %d"), 34);
	UpdateTileParameters(CanvasPosition, MousePosition);
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	return HasRoomForItem(Item->GetItemManifest());
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

bool UInv_InventoryGrid::IsUpperLeftSlot(const UInv_GridSlots* GridSlot, const UInv_GridSlots* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetIndex();
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlots* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlots* GridSlot, const UInv_GridSlots* SubGridSlot,
                                              const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
                                              const FGameplayTag& ItemTypes, const int32 MaxStackSize) const
{
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetIndex())) return false;

	// Has valid item?
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		return true;
	}
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot)) return false;
	const UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable()) return false;
	if (!DoesItemTypeMatch(SubItem, ItemTypes)) return false;
	if (GridSlot->GetStackCount() >= MaxStackSize) return false;

	return true;
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num()) return false;
	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;
	return EndColumn <= Columns && EndRow <= Rows;
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass);
	}
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<
		FInv_ImageFragment>(InventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}

void UInv_InventoryGrid::RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex)
{
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	if (!GridFragment) return;
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns,
	                                 [&](UInv_GridSlots* GridSlot)
	                                 {
		                                 GridSlot->SetInventoryItem(nullptr);
		                                 GridSlot->SetUpperLeftIndex(INDEX_NONE);
		                                 GridSlot->SetUnoccupiedTexture();
		                                 GridSlot->SetAvailable(true);
		                                 GridSlot->SetStackCount(0);
	                                 });

	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex,
                                         const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);
	HoverItem->SetPreviousGridIndex(PreviousGridIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

void UInv_InventoryGrid::PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	AssignHoverItem(ClickedInventoryItem, GridIndex, GridIndex);
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
}

void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get())) return;

	for (const auto& Availability : Result.SlotAvailabilities)
	{
		if (Availability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			UE_LOG(LogTemp, Warning, TEXT("The float value: %d"), Availability.AmountTofill);
			UE_LOG(LogTemp, Warning, TEXT("The float value123: %d"), GridSlot->GetStackCount());
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availability.AmountTofill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + Availability.AmountTofill);
		}
		else
		{
			AddItemAtIndex(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountTofill);
			UpdateGridSlots(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountTofill);
		}
	}
}

void UInv_InventoryGrid::OnSlottedItemClickec(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	//	UE_LOG(LogTemp, Warning, TEXT("Cliecked on item at index %d"),1);
	check(GridSlots.IsValidIndex(GridIndex));
	UInv_InventoryItem* ClickedInventoyItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	// UE_LOG(LogTemp, Warning, TEXT("Cliecked on item at index %d"),GridIndex);
	if (!IsValid(HoverItem) && IsLeftClick(MouseEvent))
	{
		PickUp(ClickedInventoyItem, GridIndex);
  return;
	}
	if (IsRightClick(MouseEvent))
	{
		CreateItemPopUp(GridIndex);
		return;
	}
	if (IsSameStackable(ClickedInventoyItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoyItem->GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		UE_LOG(LogTemp, Warning, TEXT("The roomin luliul: %d"), RoomInClickedSlot);
		const int32 HoveredStackCount = HoverItem->GetStackCount();
		
		// Should we swap their stack counts? (Room in the clicked slot == 0 && HoveredStackCount < MaxStackSize)
		if (shouldSwapStackCount(RoomInClickedSlot, HoveredStackCount, MaxStackSize))
		{
			SwapStackCounts(ClickedStackCount,HoveredStackCount,GridIndex);
return;
		}
		if (ShouldConsumeHoverItemStacks(HoveredStackCount,RoomInClickedSlot))
		{
			ConsumerHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex);	
return;
		}
		if (ShouldfillInStacks(RoomInClickedSlot, HoveredStackCount))
		{
			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return;
		}
		if (RoomInClickedSlot == 0)
		{
			return;
		}
	}

	SwapWithHoverItem(ClickedInventoyItem,GridIndex);
}

void UInv_InventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	UInv_GridSlots* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);

	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

	HoverItem->UpdateStackCount(Remainder);
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlots* GridSlot, const FIntPoint& Dimensions,
                                        const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed,
                                        const FGameplayTag& ItemTypes, const int32 MaxStackSize)
{
	bool bHasRoomAtIndex = true;

	UInv_InventoryStatics::ForEach2D(GridSlots, GridSlot->GetIndex(), Dimensions, Columns,
	                                 [&](const UInv_GridSlots* SubGridSlot)
	                                 {
		                                 if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices,
		                                                          OutTentativelyClaimed, ItemTypes, MaxStackSize))
		                                 {
			                                 OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		                                 }
		                                 else
		                                 {
			                                 bHasRoomAtIndex = false;
		                                 }
	                                 });

	return bHasRoomAtIndex;
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& Manifest) const
{
	const FInv_GridFragment* GridFragment = Manifest.GetFragmentOfType<FInv_GridFragment>();
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
}


FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest)
{
	FInv_SlotAvailabilityResult Result;
	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1;

	TSet<int32> CheckedIndices;
	//	UE_LOG(LogTemp, Warning, TEXT("The float value is luliul: %d"), AmountToFill);
	for (const auto& GridSlot : GridSlots)
	{
		// If we don't have anymore to fill, break out of the loop early.
		if (AmountToFill == 0) break;
		//UE_LOG(LogTemp, Warning, TEXT("The float value is luliubreak: %d"), AmountToFill);
		if (IsIndexClaimed(CheckedIndices, GridSlot->GetIndex())) continue;
		//UE_LOG(LogTemp, Warning, TEXT("The float value is luliubreak123: %d"), AmountToFill);
		if (!IsInGridBounds(GridSlot->GetIndex(), GetItemDimensions(Manifest))) continue;
		//UE_LOG(LogTemp, Warning, TEXT("The float value is luliubreak12345: %d"), AmountToFill);
		TSet<int32> TentativelyClaimed;
		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(Manifest), CheckedIndices, TentativelyClaimed,
		                    Manifest.GetItemType(), MaxStackSize))
		{
			continue;
		}
		UE_LOG(LogTemp, Warning, TEXT("The float value is luliubreak12345789: %d"), AmountToFill);
		//	CheckedIndices.Append(TentativelyClaimed);

		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill,
		                                                            GridSlot);
		//	UE_LOG(LogTemp, Warning, TEXT("The float value is luliul12345678: %d"), AmountToFillInSlot);
		if (AmountToFillInSlot == 0) continue;
		CheckedIndices.Append(TentativelyClaimed);
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			FInv_SlotAvailability{
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetIndex(),
				Result.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);

		AmountToFill -= AmountToFillInSlot;

		// How much is the Remainder?
		Result.Remainder = AmountToFill;

		if (AmountToFill == 0) return Result;
	}
	//	UE_LOG(LogTemp, Warning, TEXT("The float value is luliul123456: %d"), Result.TotalRoomToFill);

	//	UE_LOG(LogTemp, Warning, TEXT("The float value is luliul1234: %d"), AmountToFill);
	return Result;
}

void UInv_InventoryGrid::OnTileParametersUpdate(const FInv_TileParameters& Parameters)
{
	if (!IsValid(HoverItem)) return;
	//UE_LOG(LogTemp, Warning, TEXT("The flo : %d"), 34);
	const FIntPoint Dimensions = HoverItem->GetGridDimensions();

	const FIntPoint StartingCoordinate = CalculateStartingCoordinate(Parameters.TileCoordinats, Dimensions,
	                                                                 Parameters.TileQuadrant);
	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinate,Columns);

	CurrentQueryResult = CheckHoverPosition(StartingCoordinate, Dimensions);
  if (CurrentQueryResult.bHasSpace)
  {
  //	UE_LOG(LogTemp, Warning, TEXT("The float : %d"), 34);
	  HighLight(ItemDropIndex,Dimensions);
  	return;
  }
	UnHighLight(LastHighlightIndex,LastHighlightedDimensions);

	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		// TODO: There's a single item in this space. We can swap or add stacks.
		const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(CurrentQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment) return;

		ChangeHoverType(CurrentQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EInv_GridSlotState::GreyedOut);
	}
}

void UInv_InventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	if (!bMouseWithinCanvas) return;
	const FIntPoint HoveredTileCoordinates = CalculateHoveredCoordinates(CanvasPosition, MousePosition);

	LastTileParameters = TileParameters;
	TileParameters.TileCoordinats = HoveredTileCoordinates;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(HoveredTileCoordinates, Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition, MousePosition);
	OnTileParametersUpdate(TileParameters);
}



void UInv_InventoryGrid::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions,
	EInv_GridSlotState GridSlotState)
{
	UnHighLight(LastHighlightIndex, LastHighlightedDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [State = GridSlotState](UInv_GridSlots* GridSlot)
	{
		switch (State)
		{
		case EInv_GridSlotState::Occupied:
			GridSlot->SetOccupiedTexture();
			break;
		case EInv_GridSlotState::Unoccupied:
			GridSlot->SetUnoccupiedTexture();
			break;
		case EInv_GridSlotState::GreyedOut:
			GridSlot->SetGreyedOutTexture();
			break;
		case EInv_GridSlotState::Selected:
			GridSlot->SetSelectedTexture();
			break;
		}
	});

	LastHighlightIndex= Index;
	LastHighlightedDimensions = Dimensions;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,
                                                     const int32 AmountToFill, const UInv_GridSlots* GridSlot) const
{
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1;
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlots* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	// If we are at a slot that doesn't hold the stack count. we must get the actual stack count.
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlots* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftGridSlot->GetStackCount();
	}
	return CurrentSlotStackCount;
}

UUserWidget* UInv_InventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(),VisibleCursorWidgetClass);
	}
	return VisibleCursorWidget;
}

UUserWidget* UInv_InventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(),VisibleCursorWidgetClass);
	}
	return HiddenCursorWidget;
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	//UE_LOG(LogTemp, Warning, TEXT("The float value isgvg: %d"), Item->GetItemManifest().GetItemCategory());
	//UE_LOG(LogTemp, Warning, TEXT("The float value is g g: %d"), ItemCategory);

	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}

void UInv_InventoryGrid::ConstrcutGrid()
{
	GridSlots.Reserve(Rows * Columns);
	for (int32 j = 0; j < Rows; j++)
	{
		for (int32 i = 0; i < Columns; i++)
		{
			UInv_GridSlots* GridSlot = CreateWidget<UInv_GridSlots>(this, GridSlotClass);
			CanvasPanel->AddChild(GridSlot);

			const FIntPoint TilePosition(i, j);
			GridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(TilePosition, Columns));

			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCPS->SetSize(FVector2D(TileSize));
			GridCPS->SetPosition(TilePosition * TileSize);

			GridSlots.Add(GridSlot);
			GridSlot->GridSlotClicked.AddDynamic(this, &ThisClass::OnGridSlotClicked);
			GridSlot->GridSlotHovered.AddDynamic(this, &ThisClass::OnGridSlotHovered);
			GridSlot->GridSlotUnhovered.AddDynamic(this, &ThisClass::OnGridSlotUnhovered);
		}
	}
}

void UInv_InventoryGrid::PutDownOnIndex(const int32 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("The : %d"), HoverItem->IsStackable());
	UE_LOG(LogTemp, Warning, TEXT("The : %d"), HoverItem->GetStackCount());
	AddItemAtIndex(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	UpdateGridSlots(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	ClearHoverItem();
}

void UInv_InventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem)) return;

	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetPreviousGridIndex(INDEX_NONE);
	HoverItem->UpdateStackCount(0);
	HoverItem->SetImageBrush(FSlateNoResource());

	HoverItem->RemoveFromParent();
	HoverItem = nullptr;
	ShowCursor();
}

void UInv_InventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem)) return;
	if(!GridSlots.IsValidIndex(ItemDropIndex)) return;
	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		OnSlottedItemClickec(CurrentQueryResult.UpperLeftIndex,MouseEvent);
  return;
	}

	auto GridSlot = GridSlots[ItemDropIndex];
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		  PutDownOnIndex(ItemDropIndex);
	}
}

void UInv_InventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;

	UInv_GridSlots* GridSlot = GridSlots[GridIndex];
	if (GridSlot->isAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInv_InventoryGrid::OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;

	UInv_GridSlots* GridSlot = GridSlots[GridIndex];
	if (GridSlot->isAvailable())
	{
		GridSlot->SetUnoccupiedTexture();
	}
}
FIntPoint UInv_InventoryGrid::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
                                                          EInv_TileQuadrant Quadrant) const
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoord;
	switch (Quadrant)
	{
	case EInv_TileQuadrant::TopLeft:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case EInv_TileQuadrant::TopRight:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case EInv_TileQuadrant::BottomLeft:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	case EInv_TileQuadrant::BottomRight:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	default:
		UE_LOG(MyLogCat, Error, TEXT("Invalid Quadrant."))
		return FIntPoint(-1, -1);
	}
	return StartingCoord;
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
                                                UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	//	UE_LOG(LogTemp, Warning, TEXT("The float value X: %f"), DrawPos.X);
	//	UE_LOG(LogTemp, Warning, TEXT("The float value.Y: %f"), DrawPos.Y);
	//	UE_LOG(LogTemp, Warning, TEXT("The float value.Tile: %f"), TileSize);
	const FVector2D DrawPosWithPAdding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	//	UE_LOG(LogTemp, Warning, TEXT("The float value padding X: %f"), DrawPosWithPAdding.X);
	//	UE_LOG(LogTemp, Warning, TEXT("The float value.padding Y: %f"), DrawPosWithPAdding.Y);
	CanvasSlot->SetPosition(DrawPosWithPAdding);
}
