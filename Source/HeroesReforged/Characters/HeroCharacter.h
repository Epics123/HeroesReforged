// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroCharacter.generated.h"

class USpringArmComponent;
class UHeroCameraComponent;
class UHeroMovementComponent;
class AHeroAIController;

struct FInputActionValue;

UENUM()
enum class EHeroType : uint8
{
	Speed,
	Power,
	Flight
};

UCLASS()
class HEROESREFORGED_API AHeroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHeroCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure)
	UHeroMovementComponent* GetHeroMovementComponent() const;

	UFUNCTION(BlueprintPure)
	EHeroType GetHeroType() const { return HeroType; }

	UFUNCTION(BlueprintPure)
	bool IsMoveInputBlocked() const;

protected:
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void MoveReleased();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;

	void SwapLeft();
	void SwapRight();

	virtual void PossessedBy(AController* NewController) override;
	virtual void PawnClientRestart() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void SwapHeroInternal(int32 Direction);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHeroType HeroType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AHeroAIController> AIController;

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeroCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USceneComponent* CameraPivot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPlayerInputData> InputData;
};
