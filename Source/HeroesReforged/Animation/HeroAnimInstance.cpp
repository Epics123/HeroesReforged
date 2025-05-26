// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAnimInstance.h"
#include "../Characters/HeroCharacter.h"
#include "../Components/HeroMovementComponent.h"

UHeroAnimInstance::UHeroAnimInstance()
{

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
