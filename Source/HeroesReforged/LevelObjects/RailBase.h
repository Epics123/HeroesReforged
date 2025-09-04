// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RailBase.generated.h"

class USplineComponent;
class UStaticMesh;
class UCapsuleComponent;
class AHeroCharacter;

UCLASS()
class HEROESREFORGED_API ARailBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARailBase();

	UFUNCTION(BlueprintPure)
	FVector GetClosestPointOnRail();

	UFUNCTION(BlueprintPure)
	FQuat GetOrientationOnRail();

	UFUNCTION(BlueprintCallable)
	void SetRailActiveState(bool bActive);

	UFUNCTION(BlueprintGetter)
	bool IsRailActive() const { return bRailActive; }

	UFUNCTION(BlueprintCallable)
	bool AttachToRail();

	UFUNCTION(BlueprintCallable)
	void DetachFromRail();

	UFUNCTION(BlueprintPure)
	float GetDirectionOnRail();

	UFUNCTION(BlueprintGetter)
	FVector GetVelocityProjectedOnRail();

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerVelocity(float DeltaTime, const FVector& ProjectedVelocity, bool bHorrizontalOverride = false, bool bVerticalOverride = false, bool bUseAsMultiplier = false);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateTransformOnRail();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
#if WITH_EDITOR
	void VisualizeDebug();
#endif

	FVector GetRailTangentAtPlayerLocation() const;

	float GetSlopeAcceleration(const FVector& Tangent) const;

public:
	/** Spline component to define the rail path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rail")
	TObjectPtr<USplineComponent> RailSpline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rail")
	TObjectPtr<UStaticMesh> RailMesh;

	/** Vertical offset of character on the rail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float RailVerticalOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float RailSwitchTraceDistance = 200.0f;

	/** Whether or not to allow player input to move character backwards on the rail*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Movement")
	bool bOneWay = true;

	/** If true, only allow forward movement along the rail (i.e. no rollback down a hill)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Movement")
	bool bForceForwardOnly = false;

	/** Minimum rail speed when bForceForwardOnly is true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Movement")
	float MinRailSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Movement")
	float SlopeAccelerationMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	TObjectPtr<AHeroCharacter> PlayerRef;

private:
	bool bRailActive;
};
