// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HeroesReforgedGameMode.generated.h"

class UHeroManager;

UCLASS(minimalapi)
class AHeroesReforgedGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHeroesReforgedGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroManagement")
	TObjectPtr<UHeroManager> HeroManager;
};



