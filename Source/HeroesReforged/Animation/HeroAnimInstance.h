// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HeroAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HEROESREFORGED_API UHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UHeroAnimInstance();

	UFUNCTION(BlueprintCallable)
	void CalculateSpeedAndDirection();

	UFUNCTION(BlueprintCallable)
	void TryGetHero();

protected:
	void NativeBeginPlay() override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	TObjectPtr<class AHeroCharacter> Hero;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float IdleTransitionThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TObjectPtr<UCurveFloat> LeanCurve;
};
