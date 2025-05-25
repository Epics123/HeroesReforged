// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "HeroCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class HEROESREFORGED_API AHeroCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AHeroCameraManager(const FObjectInitializer& ObjectInitializer);
};
