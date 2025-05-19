// Fill out your copyright notice in the Description page of Project Settings.

#include "HeroMovementComponent.h"

#include "../Characters/HeroCharacter.h"

UHeroMovementComponent::UHeroMovementComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bConstrainToPlane = false;
	bOrientRotationToMovement = false;
	bMaintainHorizontalGroundVelocity = false;
}

void UHeroMovementComponent::SetCustomGravityDir(const FVector& Dir)
{
	CustomGravityDirection = Dir.GetSafeNormal();
}

void UHeroMovementComponent::ResetGravity()
{
	PreviousNormal = FVector(0.0f, 0.0f, 1.0f);
	CustomGravityDirection = FVector(0.0f, 0.0f, -1.0f);
	SetGravityDirection(CustomGravityDirection); // TODO: Handle overriding gravity (i.e. if we want to force character to fall in a certain direction)
}

void UHeroMovementComponent::UpdateFlightTime(float DeltaTime)
{
	CurrentFlightTime -= DeltaTime;
}

float UHeroMovementComponent::CalculateVerticalDirection() const
{
	if (CharacterOwner)
	{
		if (bFlightExhausted)
		{
			return -1.0f;
		}

		const FVector CurrentLocation = CharacterOwner->GetActorLocation();
		const FVector MaxFlightLocation = FVector(CurrentLocation.X, CurrentLocation.Y, MaxFlightZ);
		const float Dist = (MaxFlightLocation - CurrentLocation).Size();

		return FMath::GetMappedRangeValueClamped(FVector2D(0.0f, MaxFlightZ), FVector2D(0.0f, 1.0f), Dist);
	}

	return 0.0f;
}

float UHeroMovementComponent::GetApexProximity(float ApexHeight, float JumpStartHeight) const
{
	const float VerticalVelocity = RotateWorldToGravity(Velocity).Z;
	if (VerticalVelocity > 0.0f)
	{
		return 1.0f - FMath::Clamp(FMath::Abs(RotateWorldToGravity(Velocity).Z / JumpZVelocity), 0.0f, 1.0f);
	}
	else if (ApexHeight > JumpStartHeight)
	{
		const float CurrentHeight = CharacterOwner->GetActorLocation().Z;
		return 1.0f - FMath::Clamp((ApexHeight - CurrentHeight) / (ApexHeight - JumpStartHeight), 0.0f, 1.0f);
	}

	return 0.0f;
}

void UHeroMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	DefaultMaxSpeed = MaxWalkSpeed;
}

void UHeroMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHeroMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	if (!CharacterOwner || !CharacterOwner->Controller)
	{
		return;
	}

	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = Start + CustomGravityDirection * GroundTraceDistance;

	if (GetWorld()->LineTraceSingleByChannel(CustomFloorHit, Start, End, ECC_Visibility))
	{
		CustomGravityDirection = -CustomFloorHit.Normal;
		CurrentNormal = CustomFloorHit.Normal;
		SetGravityDirection(CustomGravityDirection);
	}

	const FVector SmoothedNormal = FMath::VInterpTo(PreviousNormal, CurrentNormal, DeltaTime, NormalSmoothingSpeed);
	PreviousNormal = SmoothedNormal;

	UpdateOwnerRotation(SmoothedNormal, DeltaTime);
	CalculateTangentVelocity(SmoothedNormal, DeltaTime);

	if (Velocity.Size() < SlopeEffectThreshold)
	{
		// TODO: Slope effect should work with custom gravity

		const FVector VelocityDir = Velocity.GetSafeNormal();
		const FVector SlopeDir = FVector::VectorPlaneProject(-SmoothedNormal, FVector(0.0f, 0.0f, 1.0f)).GetSafeNormal();

		const float SlopeSign = FMath::Sign(VelocityDir | SlopeDir);
		SlopeFactor = 1.0f - (SmoothedNormal | FVector(0.0f, 0.0f, 1.0f));

		SlopeAdjustmentSpeed = -(VelocityDir | SlopeDir) * SlopeFactor * SlopeEffectStrength;
		Velocity += VelocityDir * SlopeAdjustmentSpeed;
	}

	if (FMath::Abs(Velocity.Size()) < MinSlopeFallSpeed && (SmoothedNormal | FVector(0.0f, 0.0f, 1.0f)) <= 0.0f)
	{
		PreviousNormal = FVector(0.0f, 0.0f, 1.0f);
		CustomGravityDirection = FVector(0.0f, 0.0f, -1.0f);
		SetGravityDirection(CustomGravityDirection);

		SetMovementMode(MOVE_Falling);
		Velocity += CustomGravityDirection * 200.0f;

		return;
	}

	Super::PhysWalking(DeltaTime, Iterations);
}

void UHeroMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < SMALL_NUMBER)
	{
		return;
	}

	const FVector CustomGravityForce = CustomGravityDirection * CustomGravityStrength;
	Velocity += CustomGravityForce * DeltaTime;

	if (!GetLastInputVector().IsNearlyZero())
	{
		UpdateOwnerRotationFalling(DeltaTime);
	}

	Super::PhysFalling(DeltaTime, Iterations);
}

void UHeroMovementComponent::PhysFlying(float DeltaTime, int32 Iterations)
{
	Super::PhysFlying(DeltaTime, Iterations);

	if (bFlightInputHeld && !bFlightExhausted)
	{
		const FVector FlightDir = FVector(GetLastInputVector().X, GetLastInputVector().Y, 1.0f).GetSafeNormal();
		Velocity += (FlightDir * VerticalFlightSpeed) * DeltaTime;

		if (CharacterOwner->GetActorLocation().Z > MaxFlightZ)
		{
			Velocity.Z = 0.0f;

			// Smoothly interpolate to the correct height if our velocity takes us past it.
			const FVector CurrentLocation = CharacterOwner->GetActorLocation();
			FVector UpdatedLocation = FMath::VInterpTo(CurrentLocation, FVector(CurrentLocation.X, CurrentLocation.Y, MaxFlightZ), DeltaTime, MaxFlightHeightSmoothSpeed);
			CharacterOwner->SetActorLocation(UpdatedLocation);
		}

		if (!GetLastInputVector().IsNearlyZero())
		{
			UpdateOwnerRotationFalling(DeltaTime);
		}

		CurrentFlightTime -= DeltaTime;
		if (CurrentFlightTime <= 0.0f)
		{
			bFlightExhausted = true;
			CurrentFlightTime = 0.0f;
			OnFlightExhauseted.Broadcast();
		}
	}
}

void UHeroMovementComponent::StartFalling(int32 Iterations, float RemainingTime, float TimeTick, const FVector& Delta, const FVector& SubLoc)
{
	ResetGravity();

	Super::StartFalling(Iterations, RemainingTime, TimeTick, Delta, SubLoc);
}

bool UHeroMovementComponent::IsWalkable(const FHitResult& Hit) const
{
	//return Super::IsWalkable(Hit);
	return true;
}

bool UHeroMovementComponent::IsMovingOnGround() const
{
	return CustomFloorHit.bBlockingHit && IsWalkable(CustomFloorHit) && UpdatedComponent;
}

void UHeroMovementComponent::FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult) const
{
	const FVector Start = CapsuleLocation;
	const FVector End = Start + CustomGravityDirection * GroundTraceDistance;

	FHitResult FloorHit;
	if (GetWorld()->LineTraceSingleByChannel(FloorHit, Start, End, ECC_Visibility))
	{
		OutFloorResult.bBlockingHit = true;
		OutFloorResult.bWalkableFloor = true;
		OutFloorResult.FloorDist = FloorHit.Distance;
		OutFloorResult.HitResult = FloorHit;
	}
	else
	{
		OutFloorResult.Clear();
	}
}

void UHeroMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

bool UHeroMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const
{
	return Super::IsValidLandingSpot(CapsuleLocation, Hit);
}

bool UHeroMovementComponent::DoJump(bool bReplayingMoves)
{
	if (CharacterOwner && CharacterOwner->CanJump())
	{
		const FVector JumpDirection = GetCurrentSurfaceNormal();
		const FVector TangentVelocity = Velocity - (Velocity | GetCurrentSurfaceNormal()) * GetCurrentSurfaceNormal();

		Velocity = TangentVelocity + (JumpDirection * JumpZVelocity);

		ResetGravity();

		if (MovementMode != MOVE_Flying)
		{
			SetMovementMode(MOVE_Falling);
		}

		return true;
	}

	return false;
}

void UHeroMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	if (Velocity.IsZero() || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	const float WalkingFrictionFactor = WalkingFrictionFactorCurve->GetFloatValue(Velocity.Size());

	const float FrictionFactor = FMath::Max(0.f, MovementMode == MOVE_Walking ? WalkingFrictionFactor : BrakingFrictionFactor);
	Friction = FMath::Max(0.f, Friction * FrictionFactor);
	BrakingDeceleration = FMath::Max(0.f, BrakingDeceleration);
	const bool bZeroFriction = (Friction == 0.f);
	const bool bZeroBraking = (BrakingDeceleration == 0.f);

	if (bZeroFriction && bZeroBraking)
	{
		return;
	}

	const bool bIgnoreZ = MovementMode == MOVE_Flying;

	const FVector OldVel = Velocity;

	// subdivide braking to get reasonably consistent results at lower frame rates
	// (important for packet loss situations w/ networking)
	float RemainingTime = DeltaTime;
	const float MaxTimeStep = FMath::Clamp(BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);

	// Decelerate to brake to a stop
	const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (-BrakingDeceleration * Velocity.GetSafeNormal()));
	while (RemainingTime >= MIN_TICK_TIME)
	{
		// Zero friction uses constant deceleration, so no need for iteration.
		const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
		RemainingTime -= dt;

		// apply friction and braking
		Velocity = Velocity + ((-Friction) * Velocity + RevAccel) * dt;
		if (bIgnoreZ)
		{
			Velocity.Z = OldVel.Z;
		}

		// Don't reverse direction
		if ((Velocity | OldVel) <= 0.f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	const FVector LateralVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
	const float LateralVSizeSq = LateralVelocity.SizeSquared();
	if (bIgnoreZ && (LateralVSizeSq <= UE_KINDA_SMALL_NUMBER || (!bZeroBraking && LateralVSizeSq <= FMath::Square(BRAKE_TO_STOP_VELOCITY))))
	{
		Velocity = FVector(0.0f, 0.0f, Velocity.Z);
	}

	// Clamp to zero if nearly zero, or if below min threshold and braking.
	const float VSizeSq = Velocity.SizeSquared();
	if (VSizeSq <= UE_KINDA_SMALL_NUMBER || (!bZeroBraking && VSizeSq <= FMath::Square(BRAKE_TO_STOP_VELOCITY)))
	{
		Velocity = FVector::ZeroVector;
	}
}

void UHeroMovementComponent::UpdateOwnerRotation(const FVector& SurfaceNormal, float DeltaTime)
{
	const APlayerController* Controller = Cast<APlayerController>(CharacterOwner->GetController());
	if (!Controller)
	{
		return;
	}

	const FVector ForwardVector = CharacterOwner->GetActorForwardVector();

	// Align to surface
	const FQuat TargetOrientation = FRotationMatrix::MakeFromZX(SurfaceNormal, ForwardVector).ToQuat();
	FQuat SmoothTargetOrientation = FQuat::Slerp(CharacterOwner->GetActorQuat(), TargetOrientation, DeltaTime * RotationSmoothingSpeed);
	SmoothTargetOrientation.Normalize();

	bool bSkipRotateToVelocity = false;
	//const AHeroCharacter* Character = Cast<AHeroCharacter>(CharacterOwner);
	//if (Character && Character->IsAiming())
	//{
	//	bSkipRotateToVelocity = true; // Skip rotating towards velocity if we are aiming a weapon
	//}

	// Rotate towards velocity
	const FVector TangentVelocity = Velocity - (Velocity | SurfaceNormal) * SurfaceNormal;
	if (!TangentVelocity.IsNearlyZero(RotateToVelocityThreshold) && !bSkipRotateToVelocity)
	{
		const FQuat TargetForwardOrientation = FRotationMatrix::MakeFromXZ(TangentVelocity.GetSafeNormal(), SurfaceNormal).ToQuat();
		SmoothTargetOrientation = FQuat::Slerp(SmoothTargetOrientation, TargetForwardOrientation, DeltaTime * RotateToVelocityCurve->GetFloatValue(TangentVelocity.Size()));
		SmoothTargetOrientation.Normalize();
	}

	CharacterOwner->SetActorRotation(SmoothTargetOrientation);
}

void UHeroMovementComponent::UpdateOwnerRotationFalling(float DeltaTime)
{
	FVector VelocityDir = Velocity.GetSafeNormal();
	VelocityDir.Z = 0.0f;

	const FRotator TargetRotation = FRotator(0.0f, VelocityDir.Rotation().Yaw, 0.0f);
	FRotator UpdatedRotation = FMath::RInterpTo(CharacterOwner->GetActorRotation(), TargetRotation, DeltaTime, AirRotationSmoothingSpeed);

	CharacterOwner->SetActorRotation(UpdatedRotation);
}

void UHeroMovementComponent::CalculateTangentVelocity(const FVector& SurfaceNormal, float DeltaTime)
{
	const FVector ForwardVector = CharacterOwner->GetActorForwardVector();

	const FVector TangentVelocity = Velocity - (Velocity | SurfaceNormal) * SurfaceNormal;
	FVector TangetForward = TangentVelocity.GetSafeNormal();

	if (TangetForward.SizeSquared() < UE_KINDA_SMALL_NUMBER)
	{
		TangetForward = CharacterOwner->GetActorForwardVector();;
	}

	Velocity = TangentVelocity + TangetForward * TangentVelocityScale;
}

float UHeroMovementComponent::VisualizeMovement() const
{
	float HeightOffset = 0.f;
	const float OffsetPerElement = 10.0f;
	if (CharacterOwner == nullptr)
	{
		return HeightOffset;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const FVector TopOfCapsule = GetActorLocation() + FVector(0.f, 0.f, CharacterOwner->GetSimpleCollisionHalfHeight());

	// Position
	{
		const FColor DebugColor = FColor::White;
		const FVector DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
		FString DebugText = FString::Printf(TEXT("Position: %s"), *GetActorLocation().ToCompactString());
		DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + CharacterOwner->GetActorForwardVector() * 50.0f, 1.0f, FColor::Red, false, -1.0f, 0, 1.0f);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + CharacterOwner->GetActorRightVector() * 50.0f, 1.0f, FColor::Green, false, -1.0f, 0, 1.0f);
	}

	// Velocity
	{
		const FColor DebugColor = FColor::Cyan;
		HeightOffset += OffsetPerElement;
		const FVector DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + Velocity.GetSafeNormal() * 75.0f,
			1.f, DebugColor, false, -1.f, (uint8)'\000', 1.f);

		FString DebugText = FString::Printf(TEXT("Velocity: %s (Speed: %.2f) (Max: %.2f)"), *Velocity.ToCompactString(), Velocity.Size(), GetMaxSpeed());
		DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);
	}

	// Acceleration
	{
		const FColor DebugColor = FColor::Yellow;
		HeightOffset += OffsetPerElement;
		const float MaxAccelerationLineLength = 200.f;
		const float CurrentMaxAccel = GetMaxAcceleration();
		const float CurrentAccelAsPercentOfMaxAccel = CurrentMaxAccel > 0.f ? Acceleration.Size() / CurrentMaxAccel : 1.f;
		const FVector DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(),
			GetActorLocation() + Acceleration.GetSafeNormal(UE_SMALL_NUMBER) * CurrentAccelAsPercentOfMaxAccel * MaxAccelerationLineLength,
			1.f, DebugColor, false, -1.f, (uint8)'\000', 1.f);

		FString DebugText = FString::Printf(TEXT("Acceleration: %s"), *Acceleration.ToCompactString());
		DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);
	}

	// Movement Mode
	{
		FColor DebugColor = FColor::Emerald;
		HeightOffset += OffsetPerElement;
		FVector DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
		FString DebugText = FString::Printf(TEXT("MovementMode: %s"), *GetMovementName());
		DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

		HeightOffset += OffsetPerElement;
		DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
		DebugText = FString::Printf(TEXT("Gravity Direction: %s"), *CustomGravityDirection.ToCompactString());
		DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

		if (IsFlying())
		{
			DebugColor = FColor::Purple;

			HeightOffset += OffsetPerElement;
			DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
			DebugText = FString::Printf(TEXT("Max Flight Z: %.2f"), MaxFlightZ);
			DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

			const FVector MaxFlightLocation = FVector(DebugLocation.X, DebugLocation.Y, MaxFlightZ);
			DrawDebugSphere(GetWorld(), MaxFlightLocation, 5.0f, 16.0f, DebugColor);

			DrawDebugLine(GetWorld(), GetActorLocation(), MaxFlightLocation, DebugColor);
		}
		else if (!IsFalling())
		{
			DebugColor = FColor::Turquoise;
			HeightOffset += OffsetPerElement;
			DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
			DebugText = FString::Printf(TEXT("Slope Factor: %.2f"), SlopeFactor);
			DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

			DebugColor = FColor::Magenta;
			HeightOffset += OffsetPerElement;
			DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
			DebugText = FString::Printf(TEXT("Slope Adjustment Speed: %.2f"), SlopeAdjustmentSpeed);
			DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

			DebugColor = FColor::Emerald;
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CustomGravityDirection * GroundTraceDistance, DebugColor);
		}
	}

	// Jump
	{
		const FColor DebugColor = FColor::Blue;
		HeightOffset += OffsetPerElement;
		FVector DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
		FString DebugText = FString::Printf(TEXT("bIsJumping: %d Count: %d HoldTime: %.2f"), CharacterOwner->bPressedJump, CharacterOwner->JumpCurrentCount, CharacterOwner->JumpKeyHoldTime);
		DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);
	}
#endif

	return HeightOffset;
}
