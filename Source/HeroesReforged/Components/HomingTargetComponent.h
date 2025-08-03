// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "../Characters/HeroCharacter.h"

#include "HomingTargetComponent.generated.h"

UCLASS( Blueprintable, meta=(BlueprintSpawnableComponent) )
class HEROESREFORGED_API UHomingTargetComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHomingTargetComponent();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ShowHomingTarget();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void HideHomingTarget();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void HandleTargetHit(AHeroCharacter* Attacker);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	

public:
	// If true, the player can homing attack this component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowHomingAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDestroyWhenHit;

	// If true, LaunchVelocity will be used to launch the character when a homing target is hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseLaunchVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bUseLaunchVelocity"))
	FVector LaunchVelocity;

	// XY strength of deflection vector that is added to launch velocity when homing attacking a target that does not get destroyed when hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bUseLaunchVelocity"))
	float DeflectionStrength = 800.0f;
};
