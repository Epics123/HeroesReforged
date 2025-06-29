// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"

#include "Data/HeroTeamData.h"

#include "HeroManager.generated.h"


class AHeroCharacter;

DEFINE_LOG_CATEGORY_STATIC(LogHeroManager, Display, All);

DECLARE_MULTICAST_DELEGATE(FOnHeroTeamSetup);

/**
 * 
 */
UCLASS(Blueprintable)
class HEROESREFORGED_API UHeroManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeroManager(const FObjectInitializer& ObjectInitializer);

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

	UFUNCTION(BlueprintPure)
	int32 GetHeroIndex(AHeroCharacter* Hero) const
	{
		return Heroes.IndexOfByKey(ActiveHero);
	}

	UFUNCTION(BlueprintPure)
	bool IsActiveHero(AHeroCharacter* Hero)
	{
		if(Hero)
		{
			return ActiveHero == Hero;
		}

		return false;
	}

	void SetupTeam(class AController* Controller);

	FVector GetPrevHeroVelocity() const { return PrevHeroVelocity; }

	void UpdateHeroMaxSpeeds();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heroes")
	TObjectPtr<AHeroCharacter> ActiveHero;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heroes")
	TObjectPtr<UHeroTeamData> TeamData;

	FOnHeroTeamSetup OnHeroTeamSetup;

private:
	TArray<AHeroCharacter*, TFixedAllocator<3>> Heroes;

	// The previous ActiveHero
	AHeroCharacter* PreviousHero = nullptr;
	FVector PrevHeroVelocity;
};
