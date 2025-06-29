// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HeroAIController.generated.h"

/**
 * 
 */
UCLASS()
class HEROESREFORGED_API AHeroAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) {};

	virtual void OnPossess(APawn* InPawn) override;
};
