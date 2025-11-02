// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase1.h"
#include "Inv_SpatialInventory1.generated.h"

class UCanvasPanel;
class UButton;
class UWidgetSwitcher;
class UInv_InventoryGrid;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SpatialInventory1 : public UInv_InventoryBase1
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FInv_SlotAvailabilityResult  HasRoomForItem(UInv_ItemComponent* ItemComponent ) const override;
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Equippables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Craftables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Consummables;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Equippables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Consummables;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Craftables;

	UFUNCTION()
	void ShowEquippables();

	UFUNCTION()
	void ShowConsummables();
	
	UFUNCTION()
	void ShowCraftables();
	void DisableButton(UButton* button);

	void SetActiveGrid(UInv_InventoryGrid* Grid,UButton* button);

	TWeakObjectPtr<UInv_InventoryGrid> ActiveGrid;
	
};
