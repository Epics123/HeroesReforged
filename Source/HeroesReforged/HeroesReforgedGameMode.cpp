// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesReforgedGameMode.h"
#include "HeroesReforgedCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHeroesReforgedGameMode::AHeroesReforgedGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
