// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeroAIComponent.generated.h"

class AHeroesReforgedGameMode;
class UHeroManager;
class AHeroCharacter;

UENUM(BlueprintType)
enum class EHeroAIComponentState : uint8
{
	Active,
	Inactive
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEROESREFORGED_API UHeroAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHeroAIComponent();

	UFUNCTION(BlueprintCallable)
	void SetAIComponentState(EHeroAIComponentState NewState) { HeroAIState = NewState; }

	UFUNCTION(BlueprintPure)
	bool IsAIEnabled() const { return HeroAIState == EHeroAIComponentState::Active; }

	/* 
	* Gets the index into AITargetLocations that a given character will try to move towards, based on the current ActiveHero
	* 0 = Rightmost target location, 1 = Leftmost target location
	* Assumes AITargetLocations only has 2 entries (left and right)
	*/
	UFUNCTION(BlueprintPure)
	int32 GetTargetLocationIndexFromHero(AHeroCharacter* ActiveHero, AHeroCharacter* Hero);

	void MoveToTarget(float DeltaTime);

	bool IsTargetMoving(AHeroCharacter* TargetCharacter, float Tolerence = UE_KINDA_SMALL_NUMBER);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	TArray<TObjectPtr<USceneComponent>> AITargetLocations;

	// How close we can get to the target location before stopping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	float AcceptanceRadius = 15.0f;

	// How far away the AI has to be to start speeding up to catch up to the active hero
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	float CatchUpThreshold = 600.0f;

	// Character's max speed to try and catch up to target location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	float CatchUpMaxSpeed = 4000.0f;

	// If the AI is further than this distance, teleport to the target location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	float TeleportThreshold = 2000.0f;

	// Move input will be ignored if the dot product of the change in input direction is below this threshold 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Settings")
	float OverCorrectionThreshold = -0.8f;

private:
	AHeroesReforgedGameMode* GameMode;
	UHeroManager* HeroManager;

	// The hero controlled by this UHeroAIComponent
	AHeroCharacter* OwnerHero;

	float CurrentInputScale = 1.0f;

	FVector MoveDirection = FVector::ZeroVector;

	EHeroAIComponentState HeroAIState = EHeroAIComponentState::Active;
};
