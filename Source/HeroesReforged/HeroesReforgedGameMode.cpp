// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesReforgedGameMode.h"
#include "HeroesReforgedCharacter.h"
#include "UObject/ConstructorHelpers.h"

#include "HeroManager.h"
#include "Characters/HeroCharacter.h"

UE_DISABLE_OPTIMIZATION

AHeroesReforgedGameMode::AHeroesReforgedGameMode(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void AHeroesReforgedGameMode::BeginPlay()
{
	Super::BeginPlay();
	HeroManager->OnHeroTeamSetup.Broadcast();

	HeroManager->UpdateHeroMaxSpeeds();
}

void AHeroesReforgedGameMode::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	UHeroTeamData* TeamData = HeroManager->TeamData;
	if(TeamData && TeamData->TeamMemberClasses.Num() > 0)
	{
		DefaultPawnClass = HeroManager->TeamData->TeamMemberClasses[0];
	}

	Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
}

void AHeroesReforgedGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	HeroManager->ActiveHero = Cast<AHeroCharacter>(NewPlayer->GetPawn());
	HeroManager->SetupTeam(NewPlayer);
}
