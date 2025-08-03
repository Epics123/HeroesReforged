// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroLevelComponent.h"

// Sets default values for this component's properties
UHeroLevelComponent::UHeroLevelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHeroLevelComponent::ResetLevel()
{
	CurrentLevel = 0;

	OnLevelReset.Broadcast();
}

void UHeroLevelComponent::IncreaseLevel()
{
	CurrentLevel++;
	CurrentLevel = FMath::Clamp(CurrentLevel, 0, MaxLevel);

	OnLevelUp.Broadcast();
}

void UHeroLevelComponent::DecreaseLevel()
{
	CurrentLevel--;
	CurrentLevel = FMath::Clamp(CurrentLevel, 0, MaxLevel);
}

// Called when the game starts
void UHeroLevelComponent::BeginPlay()
{
	Super::BeginPlay();
}
