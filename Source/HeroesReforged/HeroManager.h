// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Data/HeroTeamData.h"

#include "HeroManager.generated.h"


class AHeroCharacter;

DEFINE_LOG_CATEGORY_STATIC(LogHeroManager, Display, All);

/**
 * 
 */
UCLASS(Blueprintable)
class HEROESREFORGED_API UHeroManager : public UObject
{
	GENERATED_BODY()

public:
	/* Changes the active hero (-1 = Left, 1 = Right). A Direction of 0 is treated as Right. Does not actually change the underlying Heroes array. */
	UFUNCTION(BlueprintCallable)
	void RotateActiveHero(int32 Direction);

	/* Sets the active hero from available Heroes*/
	UFUNCTION(BlueprintCallable)
	void SetActiveHero(int32 Index);

	UFUNCTION(BlueprintPure)
	AHeroCharacter* GetPreviousHero() const
	{
		return PreviousHero;
	}

	UFUNCTION(BlueprintCallable)
	void AIPossessPreviousHero();

	void SetupTeam(class AController* Controller);

	FVector GetPrevHeroVelocity() const { return PrevHeroVelocity; }
	FRotator GetPrevCameraRotation() const { return PrevCameraRotation; }
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heroes")
	TObjectPtr<AHeroCharacter> ActiveHero;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heroes")
	TObjectPtr<UHeroTeamData> TeamData;

private:
	TArray<AHeroCharacter*, TFixedAllocator<3>> Heroes;

	// The previous ActiveHero
	AHeroCharacter* PreviousHero = nullptr;
	FVector PrevHeroVelocity;
	FRotator PrevCameraRotation;
};
