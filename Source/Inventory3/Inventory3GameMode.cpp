// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory3GameMode.h"
#include "Inventory3Character.h"
#include "UObject/ConstructorHelpers.h"

AInventory3GameMode::AInventory3GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
