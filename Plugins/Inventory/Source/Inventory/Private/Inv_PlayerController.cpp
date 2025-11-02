// Fill out your copyright notice in the Description page of Project Settings.


#include "Inv_PlayerController.h"
#include "Inventory.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/HUD/Inv_HUDWidget.h"
#include "Interaction/Inv_Highlightable.h"
#include "InventoryManagement/Components/Inv_InventoryComponent1.h"
#include "Items/Components/Inv_ItemComponent.h"



void AInv_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(MyLogCat, Log,TEXT("BeginPlay for PlayerController123"));

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (IsValid(Subsystem))
	{
		//for(UInputMappingContext* CurrentContext:DefaultIMC){
		//Subsystem->AddMappingContext(CurrentContext, 0);

		Subsystem->AddMappingContext(DefaultIMC, 0);

	}
	InventoryComponent = FindComponentByClass<UInv_InventoryComponent1>();
		CreateHUDWidget();


}
AInv_PlayerController::AInv_PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceLength = 500.0;
	ItemTraceChannel=ECC_GameTraceChannel1;
}

void AInv_PlayerController::ToggleInventory()
{
	if (!InventoryComponent.IsValid()) return;
	InventoryComponent->ToggleInventoryMenu();
}

void AInv_PlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//	UE_LOG(LogTemp, Warning, TEXT("Started tracing a new actor."));

	TraceForItem();
}

void AInv_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(ToggleInteractAction, ETriggerEvent::Started, this, &AInv_PlayerController::ToggleInventory);
	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &AInv_PlayerController::PrimaryInteract);
}
void AInv_PlayerController::TraceForItem()
{
//UE_LOG(LogTemp, Warning, TEXT("Started tracing a new actor."))
	//UE_LOG(LogTemp, Warning, TEXT("Started tracing a new actor."));
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport)) return;
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D ViewportCenter = ViewportSize / 2.f;
	FVector TraceStart;
	FVector Forward;
	if (!UGameplayStatics::DeprojectScreenToWorld(this, ViewportCenter, TraceStart, Forward)) return;

	const FVector TraceEnd = TraceStart + Forward * TraceLength;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel);

	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	if(!ThisActor.IsValid()){
		if(IsValid(HUDWidget)) HUDWidget->HidePickupMessage();
	}

	if (ThisActor == LastActor) return;

	if (ThisActor.IsValid())
	{
	//	UE_LOG(LogTemp, Warning, TEXT("Started tracing a new actor."))
	if(UActorComponent* Highlightable= ThisActor->FindComponentByInterface(UInv_Highlightable::StaticClass()); IsValid(Highlightable)){
		IInv_Highlightable::Execute_Highlight(Highlightable);
	}
	
	
 UInv_ItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInv_ItemComponent>();
 if(!IsValid(ItemComponent)) return;

 if(IsValid(HUDWidget)) HUDWidget->ShowPickupMessage(ItemComponent->GetPickupMessage());
	}

	if (LastActor.IsValid())
	{
		if(UActorComponent* Highlightable= LastActor->FindComponentByInterface(UInv_Highlightable::StaticClass()); IsValid(Highlightable)){
		IInv_Highlightable::Execute_Unhighlight(Highlightable);
	}
		//UE_LOG(LogTemp, Warning, TEXT("Stopped tracing last actor."))
	}
}

void AInv_PlayerController::CreateHUDWidget()
{
	if(!IsLocalController()) return;
	HUDWidget = CreateWidget<UInv_HUDWidget>(this,HUDWidgetClass);
   if(IsValid(HUDWidget)){
   	HUDWidget->AddToViewport();
   }
}

void AInv_PlayerController::PrimaryInteract()
{
	UE_LOG(LogTemp, Log, TEXT("Primary Interact 1234 "));
	if (!ThisActor.IsValid()) return;

	UInv_ItemComponent* ItemComp = ThisActor->FindComponentByClass<UInv_ItemComponent>();
	if (!IsValid(ItemComp) || !InventoryComponent.IsValid()) return;
	check(ItemComp);
	InventoryComponent->TryAddItem(ItemComp);

}

