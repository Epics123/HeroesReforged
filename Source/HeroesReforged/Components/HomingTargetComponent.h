// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HomingTargetComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEROESREFORGED_API UHomingTargetComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHomingTargetComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	
};
