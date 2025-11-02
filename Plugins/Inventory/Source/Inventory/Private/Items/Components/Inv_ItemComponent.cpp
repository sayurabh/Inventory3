// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/Inv_ItemComponent.h"

#include "Net/UnrealNetwork.h"

void UInv_ItemComponent::PickedUp()
{
	OnPickedUp();
	GetOwner()->Destroy();
}

void UInv_ItemComponent::InitItemManifest(FInv_ItemManifest Manifest)
{
	ItemManifest = Manifest;
}

// Sets default values for this component's properties
UInv_ItemComponent::UInv_ItemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PickupMessage = FString("E - PickUp");

	// ...
}

void UInv_ItemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass,ItemManifest);
}




