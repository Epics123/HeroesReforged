// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HomingAttackDefinition.generated.h"

/**
 * 
 */
UCLASS()
class HEROESREFORGED_API UHomingAttackDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> DamageValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> CooldownTimes;
};
