// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/Inv_InfoMessages.h"

#include "Components/TextBlock.h"

void UInv_InfoMessages::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Text_Message->SetText(FText::GetEmpty());
	MessageHide();
}

void UInv_InfoMessages::SetMessage(const FText& Message)
{
	Text_Message->SetText(Message);

	if (!bIsMessageActive)
	{
		MessageShow();
	}
	bIsMessageActive=true;
	GetWorld()->GetTimerManager().SetTimer(Messagetimer,[this]()
	{
		MessageHide();
		bIsMessageActive=false;
	},MessageLifetime,false);
	
}
