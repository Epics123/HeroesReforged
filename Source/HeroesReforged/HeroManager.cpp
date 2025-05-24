// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroManager.h"
#include "Characters/HeroCharacter.h"
#include "Components/HeroMovementComponent.h"

#include "GameFramework/PlayerController.h"

void UHeroManager::RotateActiveHero(int32 Direction)
{
	int32 DesiredDirection;
	if(Direction >= 0)
	{
		DesiredDirection = 1;
	}
	else
	{
		DesiredDirection = -1;
	}

	const int32 CurrentIndex = Heroes.IndexOfByKey(ActiveHero);
	int32 NewIndex = (CurrentIndex + DesiredDirection) % Heroes.Num();

	SetActiveHero(NewIndex);
}

void UHeroManager::SetActiveHero(int32 Index)
{
	if(Index >= Heroes.Num() || Index < 0)
	{
		UE_LOG(LogHeroManager, Warning, TEXT("Tried to set active hero with an invalid index! Aborting."));
		return;
	}

	AHeroCharacter* DesiredHero = Heroes[Index];
	if(DesiredHero != ActiveHero)
	{
		ActiveHero = DesiredHero;
	}
}

void UHeroManager::SetupTeam(AController* Controller)
{
	if(!TeamData)
	{
		UE_LOG(LogHeroManager, Error, TEXT("Trying to set up team without a valid TeamData!"));
		return;
	}

	Heroes.Insert(ActiveHero, 0);

	const FTransform StartTransform = ActiveHero->GetActorTransform();
	const FVector SpawnRight = ActiveHero->GetActorRightVector();

	for(int32 i = 1; i < TeamData->TeamMemberClasses.Num(); i++)
	{
		UClass* HeroClass = TeamData->TeamMemberClasses[i];

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= RF_Transient;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector SpawnLocation = ActiveHero->GetActorLocation() + (SpawnRight * TeamData->InitialSpawnOffsetDistances[i - 1]);
		const FTransform SpawnTransform = FTransform(ActiveHero->GetActorRotation(), SpawnLocation);

		AHeroCharacter* Hero = Controller->GetWorld()->SpawnActor<AHeroCharacter>(HeroClass, SpawnTransform, SpawnInfo);
		Heroes.Insert(Hero, i);
	}
}
