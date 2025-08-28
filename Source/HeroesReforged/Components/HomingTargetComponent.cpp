// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingTargetComponent.h"

// Sets default values for this component's properties
UHomingTargetComponent::UHomingTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bVisualizeComponent = true;
}


// Called when the game starts
void UHomingTargetComponent::BeginPlay()
{
	Super::BeginPlay();
}
