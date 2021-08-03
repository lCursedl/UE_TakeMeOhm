// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_TakeMeOhmGameMode.h"
#include "UE_TakeMeOhmCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUE_TakeMeOhmGameMode::AUE_TakeMeOhmGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
