// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	

public:
	// If true, the player can homing attack this component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowHomingAttack = true;
};
