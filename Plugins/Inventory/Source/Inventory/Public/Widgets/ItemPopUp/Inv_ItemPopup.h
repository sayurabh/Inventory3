// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_ItemPopup.generated.h"

/**
 * 
 */

class USlider;
class USizeBox;
class UTextBlock;
class UButton;
DECLARE_EVENT_OneParam(UUserWidget, FNativeOnDestruct, UUserWidget*);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPopUpMenuSplit, int32, SplitAmount, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuDrop, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuConsume, int32, Index);
UCLASS()
class INVENTORY_API UInv_ItemPopUp : public UUserWidget
{
	GENERATED_BODY()
protected:
	//
	void NativeDestruct();
public:
	FNativeOnDestruct OnNativeDestruct;
	FVector2D GetBoxSize() const;
	void SetGridIndex(int32 Index) { GridIndex = Index; }
	int32 GetGridIndex() const { return GridIndex; }
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnInitialized() override;
	FPopUpMenuSplit OnSplit;
	FPopUpMenuDrop OnDrop;
	FPopUpMenuConsume OnConsume;
	void CollapseSplitButton() const;
	void CollapseConsumeButton() const;
	int32 GetSplitAmount() const;
	void SetSliderParams(const float Max, const float Value) const;
private:
	int32 GridIndex{INDEX_NONE};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Drop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consume;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SplitAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
	UFUNCTION()
	void SplitButtonClicked();

	UFUNCTION()
	void DropButtonClicked();

	UFUNCTION()
	void ConsumeButtonClicked();

	UFUNCTION()
	void SliderValueChanged(float Value);
   
};
