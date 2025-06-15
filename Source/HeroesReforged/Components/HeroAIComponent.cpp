// Fill out your copyright notice in the Description page of Project Settings.

#include "HeroAIComponent.h"

#include "../HeroesReforgedGameMode.h"
#include "../HeroManager.h"
#include "../Characters/HeroCharacter.h"
#include "HeroMovementComponent.h"

#include "Kismet/GameplayStatics.h"

TAutoConsoleVariable<int32> CVar_DebugAITargetLocations(
	TEXT("Hero.AI.DebugTargetLocations"),
	0,
	TEXT("Displays target locations for hero AI characters")
);

TAutoConsoleVariable<int32> CVar_DebugAIAccpetanceRadius(
	TEXT("Hero.AI.DebugAIAccpetanceRadius"),
	0,
	TEXT("Displays the acceptance radius for hero AI characters")
);

// Sets default values for this component's properties
UHeroAIComponent::UHeroAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	OwnerHero = Cast<AHeroCharacter>(GetOwner());
}


int32 UHeroAIComponent::GetTargetLocationIndexFromHero(AHeroCharacter* ActiveHero, AHeroCharacter* Hero)
{
	check(AITargetLocations.Num() == 2);

	int32 TargetLocationIndex = -1;

	if(ActiveHero && Hero)
	{
		if(ActiveHero->HeroType == EHeroType::Speed)
		{
			switch (Hero->HeroType)
			{
			case EHeroType::Power:
				return 0;
				break;
			case EHeroType::Flight:
				return 1;
				break;
			}
		}
		else if(ActiveHero->HeroType == EHeroType::Power)
		{
			switch (Hero->HeroType)
			{
			case EHeroType::Speed:
				return 0;
				break;
			case EHeroType::Flight:
				return 1;
				break;
			}
		}
		else
		{
			switch (Hero->HeroType)
			{
			case EHeroType::Speed:
				return 0;
				break;
			case EHeroType::Power:
				return 1;
				break;
			}
		}
	}

	return TargetLocationIndex;
}

void UHeroAIComponent::MoveToTarget(float DeltaTime)
{
	if(HeroManager)
	{
		AHeroCharacter* TargetCharacter = HeroManager->ActiveHero;
		if(TargetCharacter && OwnerHero)
		{
			int32 LocationIndex = GetTargetLocationIndexFromHero(TargetCharacter, OwnerHero);
			
			const FVector TargetLocation = TargetCharacter->HeroAIComponent->AITargetLocations[LocationIndex]->GetComponentLocation();
			const FVector CurrentLocation = OwnerHero->GetActorLocation();

			const FVector ToTarget = TargetLocation - CurrentLocation;

			const bool bTargetMoving = IsTargetMoving(TargetCharacter);
			const bool bCloseEnough = ToTarget.SizeSquared() < FMath::Square(AcceptanceRadius);
			const bool bCatchUp = ToTarget.SizeSquared() > FMath::Square(TargetCharacter->HeroAIComponent->CatchUpThreshold);
			const bool bShouldTeleport = ToTarget.SizeSquared() > FMath::Square(TeleportThreshold) && !bCloseEnough;

			if(!bTargetMoving && bCloseEnough)
			{
				OwnerHero->SetActorRotation(TargetCharacter->GetActorRotation());
				return;
			}

			float MaxSpeed = bCatchUp ? CatchUpMaxSpeed : TargetCharacter->GetHeroMovementComponent()->DefaultMaxSpeed;
			OwnerHero->GetHeroMovementComponent()->MaxWalkSpeed = MaxSpeed;

			if(bShouldTeleport)
			{
				OwnerHero->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
				return;
			}

			const FVector MoveDirection = ToTarget.GetSafeNormal();
			OwnerHero->AddMovementInput(MoveDirection, 1.0f);

			if (CVar_DebugAITargetLocations.GetValueOnGameThread() > 0)
			{
				DrawDebugLine(GetWorld(), TargetLocation, CurrentLocation, FColor::Green);
				DrawDebugSphere(GetWorld(), TargetLocation, 5.0f, 12, FColor::Cyan, false, -1.0f, 1);
				
			}

			if (CVar_DebugAIAccpetanceRadius.GetValueOnGameThread() > 0)
			{
				DrawDebugSphere(GetWorld(), TargetLocation, AcceptanceRadius, 12, FColor::Magenta, false, -1.0f, 1);
			}
		}
	}
}

bool UHeroAIComponent::IsTargetMoving(AHeroCharacter* TargetCharacter, float Tolerence)
{
	if(!TargetCharacter)
	{
		return false;
	}

	return !TargetCharacter->GetVelocity().IsNearlyZero();
}

// Called when the game starts
void UHeroAIComponent::BeginPlay()
{
	GameMode = Cast<AHeroesReforgedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if(GameMode)
	{
		HeroManager = GameMode->HeroManager;
	}

	if (OwnerHero)
	{
		AITargetLocations = { OwnerHero->RightAITarget, OwnerHero->LeftAITarget };
	}

	Super::BeginPlay();
}


// Called every frame
void UHeroAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	MoveToTarget(DeltaTime);
}

