// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroCharacter.generated.h"

class USpringArmComponent;
class UHeroCameraComponent;
class UHeroMovementComponent;
class UHeroAIComponent;
class UHeroLevelComponent;
class AHeroAIController;
class UNiagaraComponent;
class USphereComponent;
class UGameplayCameraComponent;
class ARailBase;

struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE(FOnHeroJumped);
DECLARE_MULTICAST_DELEGATE(FOnHeroStopJump);

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
	UHeroAIComponent* GetAIComponent() const { return HeroAIComponent; }

	UFUNCTION(BlueprintPure)
	EHeroType GetHeroType() const { return HeroType; }

	UFUNCTION(BlueprintPure)
	int GetHeroLevel() const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnLevelUp();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLevelReset();

	UFUNCTION(BlueprintPure)
	bool IsMoveInputBlocked() const;

	UFUNCTION(BlueprintCallable)
	void SetMoveInputLocked(bool bLocked);

	UFUNCTION(BlueprintCallable)
	void SetAIComponentEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable)
	void ShowJumpball(bool bAdjustCapsuleSize = true);

	UFUNCTION(BlueprintCallable)
	void HideJumpball(bool bAdjustCapsuleSize = true);

	UFUNCTION(BlueprintPure)
	float GetJumpballPitchDuringJump(float ApexProximity, float MinDownwardRotation, float MaxUpwardRotation);

	UFUNCTION(BlueprintNativeEvent)
	void StartSecondaryJumpAction();

	virtual void Jump() override;

	virtual void StopJumping() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnRailEnd(ARailBase* Rail);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSwapRail(float Direction);

	virtual bool CanCrouch() const override;

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="CanCrouch", ScriptName="CanCrouch"))
	bool K2_CanCrouch() const;

	UFUNCTION(BlueprintImplementableEvent)
	void OnCrouchPressed();

	UFUNCTION(BlueprintImplementableEvent)
	void StartRoll();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EndRoll();

	UFUNCTION(BlueprintCallable)
	void ForceStopRoll();

protected:
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void MoveReleased();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void Landed(const FHitResult& Hit) override;

	void CheckSecondaryJumpAction();

	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;

	void SwapLeft();
	void SwapRight();

	void SwapRail(const FInputActionValue& Value);

	virtual void PossessedBy(AController* NewController) override;
	virtual void PawnClientRestart() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void SwapHeroInternal(int32 Direction);

	void ToggleJumpball(bool bShow, bool bAdjustCapsule);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHeroType HeroType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AHeroAIController> AIController;

	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<USceneComponent> RightAITarget;
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<USceneComponent> LeftAITarget;

	UPROPERTY(VisibleAnywhere, Category = AI)
	TObjectPtr<UHeroAIComponent> HeroAIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Leveling)
	TObjectPtr<UHeroLevelComponent> LevelComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RailGrinding)
	bool bOnRail = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RailGrinding)
	float RailLeanDirection = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = RailGrinding)
	TObjectPtr<ARailBase> CurrentRail;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = RailGrinding)
	TObjectPtr<ARailBase> JumpRail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	bool bRolling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	bool bCrouchHeld = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	bool bForceStopRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	bool bSpindashHeld = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rolling)
	bool bCanSpindash = true;

	FOnHeroJumped OnHeroJumped;
	FOnHeroStopJump OnHeroStopJump;

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeroCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> CameraPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> JumpballPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> JumpballMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> JumpballFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPlayerInputData> InputData;

	bool bMoveInputLocked = false;
};
