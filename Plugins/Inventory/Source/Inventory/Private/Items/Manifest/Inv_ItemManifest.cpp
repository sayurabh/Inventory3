// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Manifest/Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"

UInv_InventoryItem* FInv_ItemManifest::Manifest(UObject* NewOuter)
{
	UInv_InventoryItem* InventoryItem = NewObject<UInv_InventoryItem>(NewOuter,UInv_InventoryItem::StaticClass());
	InventoryItem->SetItemManifest(*this);

	return InventoryItem;
}

void FInv_ItemManifest::SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation,
	const FRotator& SpawnRotation)
{
	if (!IsValid(PickupActorClass) || !IsValid(WorldContextObject)) return;

	AActor* SpawnedActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedActor)) return;

	// Set the item manifest, item category, item type, etc.
	UInv_ItemComponent* ItemComp = SpawnedActor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComp);
	ItemComp->InitItemManifest(*this);
}


