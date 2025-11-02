// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_InvTypes.h"
#include "Inv_InventoryBase1.generated.h"

class UInv_ItemComponent;
struct FInv_SlotAvailabilityResult;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryBase1 : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FInv_SlotAvailabilityResult  HasRoomForItem(UInv_ItemComponent* ItemComponent ) const {return FInv_SlotAvailabilityResult();};
	
};
