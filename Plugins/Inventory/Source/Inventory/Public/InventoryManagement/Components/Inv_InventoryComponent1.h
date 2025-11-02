// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagement/FastArray/Inv_FastArray.h"
#include "Inv_InventoryComponent1.generated.h"


class UInv_ItemComponent;
class UInv_InventoryBase1;
class UInv_InventoryItem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange, const FInv_SlotAvailabilityResult&, Result);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable )
class INVENTORY_API UInv_InventoryComponent1 : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,Category="Inventory")
	float DropSpawnDistanceMax =  50.f;
	UPROPERTY(EditAnywhere,Category="Inventory")
	float DropSpawnDistanceMin =  10.f;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float RelativeSpawnElevation = 70.f;
	UPROPERTY(EditAnywhere,Category="Inventory")
	float DropSpawnAngleMax =  85.f;
	UPROPERTY(EditAnywhere,Category="Inventory")
	float DropSpawnAngleMin = -85.f;
	void SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount);
   UFUNCTION(Server,Reliable)
	void Server_ConsumeITem(UInv_InventoryItem* Item);
	UFUNCTION(Server,Reliable)
	
	void Server_DropItem(UInv_InventoryItem* Item,int32 StackCount);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(Server,Reliable)
	void Server_AddNewItem(UInv_ItemComponent* ItemComponent,int32 StackCount);

	UFUNCTION(Server,Reliable)
	void Server_AddStacksToItem(UInv_ItemComponent* ItemComponent,int32 StackCount,int32 Remainder);
	// Sets default values for this component's properties
	UInv_InventoryComponent1();
	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList;
	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly,Category="Inventory")
	void TryAddItem(UInv_ItemComponent* ItemComponent);
	void AddRepSubObj(UObject* SubObj);
	void ToggleInventoryMenu();
	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FStackChange OnStackChange;
  
protected:

	
	
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void ConstructInventory();
  TWeakObjectPtr<APlayerController> OwningController;
	
	UPROPERTY()
	TObjectPtr<UInv_InventoryBase1> InventoryMenu;
	
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInv_InventoryBase1> InventoryMenuClass;
  bool bInventoryMenuOpen;
	void OpenInventoryMenu();
	void CloseInventoryMenu();
	
	
		
};
