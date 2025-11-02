// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent1.h"

#include "Blueprint/UserWidget.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/Fragment/Inv_ItemFragment.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase1.h"

void UInv_InventoryComponent1::Server_AddNewItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	UInv_InventoryItem* NewItem = InventoryList.AddEntry(ItemComponent);
	NewItem->SetTotalStackCount(StackCount);
	if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem);
	}
	ItemComponent->PickedUp();
}

void UInv_InventoryComponent1::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent,
	int32 StackCount, int32 Remainder)
{
	const FGameplayTag& ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInv_InventoryItem* Item = InventoryList.FindFirstItemByType(ItemType);
	if (!IsValid(Item)) return;

	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount);
	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
	}
	else if (FInv_StackableFragment* StackableFragment = ItemComponent->GetItemManifest().GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(Remainder);
	}
	
	// TODO: Destroy the item if the Remainder is zero.
	// Otherwise, update the stack count for the item pickup.
}

void UInv_InventoryComponent1::Server_DropItem_Implementation(UInv_InventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	SpawnDroppedItem(Item, StackCount);
}

void UInv_InventoryComponent1::SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount)
{
	const APawn* OwningPawn = OwningController->GetPawn();
	FVector RotatedForward = OwningPawn->GetActorForwardVector();
	RotatedForward = RotatedForward.RotateAngleAxis(FMath::FRandRange(DropSpawnAngleMin,DropSpawnAngleMax),FVector::UpVector);
	FVector SpawnLocation = OwningPawn->GetActorLocation()+RotatedForward+ FMath::FRandRange(DropSpawnDistanceMin, DropSpawnDistanceMax);
	SpawnLocation.Z -= RelativeSpawnElevation;
	const FRotator Rotator = FRotator::ZeroRotator;
	FInv_ItemManifest ItemManifest = Item->GetItemManifestMutable();
	if (FInv_StackableFragment* StackableFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(StackCount);
	}
	ItemManifest.SpawnPickupActor(this, SpawnLocation, Rotator);
	
	
}

void UInv_InventoryComponent1::Server_ConsumeITem_Implementation(UInv_InventoryItem* Item)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - 1;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}else{
		Item->SetTotalStackCount(NewStackCount);
	}
	if (FInv_ConsumableFragment* ConsumableFragment = Item->GetItemManifestMutable().GetFragmentOfTypeMutable<FInv_ConsumableFragment>())
	{
		ConsumableFragment->OnConsume(OwningController.Get());
	}
	
}

void UInv_InventoryComponent1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InventoryList);
}

// Sets default values for this component's properties
UInv_InventoryComponent1::UInv_InventoryComponent1():InventoryList(this)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	bInventoryMenuOpen = false;
	// ...
}


void UInv_InventoryComponent1::AddRepSubObj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_InventoryComponent1::TryAddItem(UInv_ItemComponent* ItemComponent)
{
FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);
	
	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;
	UE_LOG(LogTemp, Warning, TEXT("The float vascdwwwwwwwwwwwwwwffflue is: %d"), Result.TotalRoomToFill);
	//UE_LOG(LogTemp, Warning, TEXT("The float value is: %d"), Result.Remainder);
	//UE_LOG(LogTemp, Warning, TEXT("The float value is: %d"), Result.bStackable);
if (Result.TotalRoomToFill==0)
{
	NoRoomInInventory.Broadcast();
	return;
}
	if (Result.Item.IsValid()&&Result.bStackable)
	{
		OnStackChange.Broadcast(Result);
		Server_AddStacksToItem(ItemComponent,Result.TotalRoomToFill,Result.Remainder);
		
	}else if (Result.TotalRoomToFill>0)
	{
		
		Server_AddNewItem(ItemComponent,Result.bStackable?Result.TotalRoomToFill:0);
	}
}

void UInv_InventoryComponent1::ToggleInventoryMenu()
{
	if (bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}else
	{
		OpenInventoryMenu();
	}
}



// Called when the game starts
void UInv_InventoryComponent1::BeginPlay()
{
	Super::BeginPlay();
	ConstructInventory();
	// ...
	
}

void UInv_InventoryComponent1::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(),TEXT("Inventory Component should have a playerConstroller a sOwner"));
	if (!OwningController->IsLocalController()) return;

	InventoryMenu = CreateWidget<UInv_InventoryBase1>(OwningController.Get(),InventoryMenuClass);
	InventoryMenu->AddToViewport();
	CloseInventoryMenu();
}

void UInv_InventoryComponent1::OpenInventoryMenu()
{
	if (!IsValid(InventoryMenu)) return;
      InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen=true;

	if (!OwningController.IsValid()) return;
	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
	
	
}

void UInv_InventoryComponent1::CloseInventoryMenu()
{
	if (!IsValid(InventoryMenu)) return;
	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen=false;

	if (!OwningController.IsValid()) return;
	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}




