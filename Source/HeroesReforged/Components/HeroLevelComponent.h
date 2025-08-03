// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeroLevelComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLevelUpDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLevelResetDelegate);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEROESREFORGED_API UHeroLevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHeroLevelComponent();

	UFUNCTION(BlueprintGetter)
	float GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintCallable)
	void ResetLevel();

	UFUNCTION(BlueprintCallable)
	void IncreaseLevel();

	UFUNCTION(BlueprintCallable)
	void DecreaseLevel();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FLevelUpDelegate OnLevelUp;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FLevelResetDelegate OnLevelReset;
	
private:
	uint32 CurrentLevel;

	uint32 MaxLevel = 3;
};
