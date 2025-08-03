// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHealthComponent.h"

UEnemyHealthComponent::UEnemyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UEnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

void UEnemyHealthComponent::ApplyDamage(float Damage)
{
	CurrentHealth -= Damage;

	if(FMath::RoundToInt(CurrentHealth) <= 0)
	{
		OnHealthDepleated.Broadcast();
	}
}