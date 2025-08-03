// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthDepleatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTamageTaken, float, Damage);


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class HEROESREFORGED_API UEnemyHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyHealthComponent();

	UFUNCTION(BlueprintGetter)
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable)
	void ApplyDamage(float Damage);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FHealthDepleatedDelegate OnHealthDepleated;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTamageTaken OnDamageTaken;

private:
	float CurrentHealth;
};
