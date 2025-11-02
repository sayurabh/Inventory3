// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemComponent.generated.h"


struct FInv_ItemManifest;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable )
class INVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void PickedUp();
	UPROPERTY(Replicated,EditAnywhere,Category="Inventory")
	FInv_ItemManifest ItemManifest;;
    void InitItemManifest(FInv_ItemManifest CopyOfManifest);
	// Sets default values for this component's properties
	UInv_ItemComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	FString GetPickupMessage() const  {return PickupMessage;}
     FInv_ItemManifest GetItemManifest() const  {return ItemManifest;}
protected:
	// Called when the game starts
	UFUNCTION(BlueprintImplementableEvent,Category="Inventory")
	void OnPickedUp();

private:
	UPROPERTY(EditAnyWhere,Category="Inventory")
	FString PickupMessage;	
	// Called every frame
	

		
};
