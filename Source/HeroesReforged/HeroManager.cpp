// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroManager.h"
#include "Characters/HeroCharacter.h"
#include "Characters/HeroAIController.h"
#include "Components/HeroMovementComponent.h"
#include "Components/HeroAIComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

UHeroManager::UHeroManager(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

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

	int32 NewIndex;
	if(CurrentIndex + DesiredDirection < 0)
	{
		NewIndex = Heroes.Num() - 1;
	}
	else
	{
		NewIndex = (CurrentIndex + DesiredDirection) % Heroes.Num();
	}
	

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
		TargetHeroVelocity = ActiveHero->GetHeroMovementComponent()->Velocity;
		SwappedHeroVelocity = DesiredHero->GetHeroMovementComponent()->Velocity;

		const FVector ActiveHeroLoc = ActiveHero->GetActorLocation();
		const FRotator ActiveHeroRot = ActiveHero->GetActorRotation();

		const FVector DesiredHeroLoc = DesiredHero->GetActorLocation();
		const FRotator DesiredHeroRot = DesiredHero->GetActorRotation();

		DesiredHero->SetActorLocationAndRotation(ActiveHeroLoc, ActiveHeroRot, false, nullptr, ETeleportType::TeleportPhysics);
		ActiveHero->SetActorLocationAndRotation(DesiredHeroLoc, DesiredHeroRot, false, nullptr, ETeleportType::TeleportPhysics);

		PreviousHero = ActiveHero;
		ActiveHero = DesiredHero;

		// We do not want the AI component to tick if we are the active character.
		ActiveHero->SetAIComponentEnabled(false);
		PreviousHero->SetAIComponentEnabled(true);

		UpdateHeroMaxSpeeds();
	}
}

void UHeroManager::AIPossessPreviousHero()
{
	if(PreviousHero && PreviousHero->AIController)
	{
		PreviousHero->AIController->Possess(PreviousHero);
	}
}

void UHeroManager::OnAcitveHeroJumped()
{
	for (AHeroCharacter* Hero : Heroes)
	{
		if(Hero != ActiveHero)
		{
			Hero->Jump();
		}
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
	ActiveHero->SetAIComponentEnabled(false);

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
		if(Hero->AIController)
		{
			Hero->AIController->Possess(Hero);
			Hero->SetAIComponentEnabled(true);
		}
		Heroes.Insert(Hero, i);
	}

	for(AHeroCharacter* Hero : Heroes)
	{
		if(Hero)
		{
			OnHeroTeamSetup.AddUObject(Hero->GetHeroMovementComponent(), &UHeroMovementComponent::CacheMovementDefaults);
		}
	}
}

void UHeroManager::UpdateHeroMaxSpeeds()
{
	for(AHeroCharacter* Hero : Heroes)
	{
		UHeroMovementComponent* MovementComponent = Hero->GetHeroMovementComponent();
		if(Hero != ActiveHero)
		{
			MovementComponent->MaxWalkSpeed = ActiveHero->GetHeroMovementComponent()->DefaultMaxSpeed;
			MovementComponent->MaxAcceleration = ActiveHero->GetHeroMovementComponent()->MaxAcceleration;
		}
		else
		{
			// Reset active hero max speed if it has changed
			if(MovementComponent->MaxWalkSpeed != MovementComponent->DefaultMaxSpeed)
			{
				MovementComponent->MaxWalkSpeed = MovementComponent->DefaultMaxSpeed;
				MovementComponent->MaxAcceleration = MovementComponent->DefaultGroundAcceleration;
			}
		}
	}
}

void UHeroManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Move hero AI characters
	for(AHeroCharacter* Hero : Heroes)
	{
		UHeroAIComponent* AIComponent = Hero->HeroAIComponent;
		if(AIComponent && AIComponent->IsAIEnabled())
		{
			AIComponent->MoveToTarget(DeltaTime);
		}
	}
}
