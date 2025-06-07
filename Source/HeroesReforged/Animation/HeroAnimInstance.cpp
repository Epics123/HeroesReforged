// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAnimInstance.h"
#include "../Characters/HeroCharacter.h"
#include "../Components/HeroMovementComponent.h"

UE_DISABLE_OPTIMIZATION

UHeroAnimInstance::UHeroAnimInstance()
{

}

void UHeroAnimInstance::NativeBeginPlay()
{
	TryGetHero();

	Super::NativeBeginPlay();
}

void UHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if(!Hero)
	{
		TryGetHero();
	}
	
	CalculateSpeedAndDirection();

	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

void UHeroAnimInstance::CalculateSpeedAndDirection()
{
	if(Hero && LeanCurve)
	{
		UHeroMovementComponent* MovementComp = Hero->GetHeroMovementComponent();

		Speed = MovementComp->Velocity.Length();

		const FVector VelocityDir = MovementComp->Velocity.GetSafeNormal();
		const float LeanValue = LeanCurve->GetFloatValue(Speed);
		
		Direction = FMath::Clamp((VelocityDir | Hero->GetActorRightVector()) * LeanValue, -1.0f, 1.0f);
	}
}

void UHeroAnimInstance::TryGetHero()
{
	APawn* PawnOwner = TryGetPawnOwner();
	if (PawnOwner)
	{
		Hero = Cast<AHeroCharacter>(PawnOwner);
	}

	USkeletalMeshComponent* OwnerComponent = GetSkelMeshComponent();
	if (AActor* OwnerActor = OwnerComponent->GetOwner())
	{
		//return Cast<APawn>(OwnerActor);
	}
}
