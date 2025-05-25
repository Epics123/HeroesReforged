// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroPlayerController.h"
#include "../Camera/HeroCameraManager.h"

#include "GameFramework/PlayerState.h"

AHeroPlayerController::AHeroPlayerController()
{

}

void AHeroPlayerController::OnPossess(APawn* InPawn)
{
	const FRotator CachedControlRotation = GetControlRotation();

	Super::OnPossess(InPawn);

	SetControlRotation(CachedControlRotation);
}
