// Copyright Epic Games, Inc. All Rights Reserved.

#include "VTTGameMode.h"
#include "VTTCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVTTGameMode::AVTTGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
