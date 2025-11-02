// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Fragment/Inv_ItemFragment.h"

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Health Potion consumed! Healing by: %f"), HealAmount));
}

void FInv_ManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Mana Potion consumed! Mana replenished by: %f"), ManaAmount));
}
