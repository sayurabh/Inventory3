// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Inv_PlayerController.generated.h"

/**
 * 
 */

class UInv_InventoryComponent1;
class UInputMappingContext;
class UInputAction;
class UInv_HUDWidget;

UCLASS()
class INVENTORY_API AInv_PlayerController : public APlayerController
{
	GENERATED_BODY()

	public:
	AInv_PlayerController();
	UFUNCTION(BlueprintCallable,Category="Inventory")
	void ToggleInventory();
	
	virtual void Tick(float DeltaTime) override;

	protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	private:

	TWeakObjectPtr<UInv_InventoryComponent1> InventoryComponent;
	void PrimaryInteract();
	void CreateHUDWidget();
	void TraceForItem();

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputMappingContext> DefaultIMC;

		//TArray<TObjectPtr<UInputMappingContext>> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> ToggleInteractAction;
		UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInv_HUDWidget> HUDWidgetClass;

		UPROPERTY()
	TObjectPtr<UInv_HUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	double TraceLength;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	TWeakObjectPtr<AActor> ThisActor;
	TWeakObjectPtr<AActor> LastActor;
	
	
};
