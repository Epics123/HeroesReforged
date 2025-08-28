// Fill out your copyright notice in the Description page of Project Settings.

#include "RailBase.h"
#include "../Characters/HeroCharacter.h"
#include "../Components/HeroMovementComponent.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

TAutoConsoleVariable<int32> CVarDebugRails(
	TEXT("Gameplay.Debug.Rails"),
	0,
	TEXT("If nonzero, displays debug info for grindable rails")
);

// Sets default values
ARailBase::ARailBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RailSpline"));
	RailSpline->SetupAttachment(RootComponent);
}

FVector ARailBase::GetClosestPointOnRail()
{
	AHeroCharacter* Character = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(Character && RailSpline)
	{
		const FVector PlayerLocation = Character->GetActorLocation();
		const FVector LocationOnSpline = RailSpline->FindLocationClosestToWorldLocation(PlayerLocation, ESplineCoordinateSpace::World);
		const FVector SplineUp = RailSpline->FindUpVectorClosestToWorldLocation(PlayerLocation, ESplineCoordinateSpace::World);

		return LocationOnSpline + (SplineUp * RailVerticalOffset);
	}
	
	return FVector::ZeroVector;
}

FQuat ARailBase::GetOrientationOnRail()
{
	if (PlayerRef && RailSpline)
	{
		const FQuat RailPointOrientation = RailSpline->FindQuaternionClosestToWorldLocation(GetClosestPointOnRail(), ESplineCoordinateSpace::World);

		const FVector RailUp = RailSpline->FindUpVectorClosestToWorldLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World);
		const FQuat DirectionalRotation = GetDirectionOnRail() < 0.0f ? FQuat::MakeFromRotationVector(RailUp * FMath::DegreesToRadians(180.0f)) : FQuat::Identity;

		return DirectionalRotation * RailPointOrientation;
	}

	return FQuat::Identity;
}

void ARailBase::SetRailActiveState(bool bActive)
{
	SetActorTickEnabled(bActive);
	bRailActive = bActive;
}

bool ARailBase::AttachToRail()
{
	bool bAttached = false;
	PlayerRef = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(PlayerRef)
	{
		UpdateTransformOnRail();

		bAttached = true;
		PlayerRef->bOnRail = bAttached;
		PlayerRef->CurrentRail = this;
	}

	return bAttached;
}

void ARailBase::DetachFromRail()
{
	if(PlayerRef)
	{
		PlayerRef->bOnRail = false;
		PlayerRef->CurrentRail = nullptr;
		PlayerRef = nullptr;
	}
}

float ARailBase::GetDirectionOnRail()
{
	float Direction = 0.0f;
	if(PlayerRef && RailSpline)
	{
		const FVector RailDirection = RailSpline->FindDirectionClosestToWorldLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World);
		Direction = RailDirection | PlayerRef->GetActorForwardVector();
	}

	return Direction;
}

FVector ARailBase::GetVelocityProjectedOnRail()
{
	FVector ProjectedVelocity = FVector::ZeroVector;
	if(PlayerRef && RailSpline)
	{
		const FVector CurrentVelocity = PlayerRef->GetHeroMovementComponent()->Velocity;
		const FVector RailDirection = GetRailTangentAtPlayerLocation();//RailSpline->FindDirectionClosestToWorldLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World);

		if (RailDirection.SizeSquared() > UE_SMALL_NUMBER)
		{
			//ProjectedVelocity = CurrentVelocity.ProjectOnTo(RailDirection);
			const float SpeedAlongRail = CurrentVelocity | RailDirection;
			ProjectedVelocity = RailDirection * SpeedAlongRail;
		}
	}

	return ProjectedVelocity;
}

void ARailBase::UpdatePlayerVelocity(float DeltaTime, const FVector& ProjectedVelocity, bool bHorrizontalOverride, bool bVerticalOverride, bool bUseAsMultiplier)
{
	if(PlayerRef)
	{
		FVector& CurrentVelocity = PlayerRef->GetHeroMovementComponent()->Velocity;
		if(bUseAsMultiplier)
		{
			CurrentVelocity *= ProjectedVelocity;
		}
		else
		{
			/*float VelX = bHorrizontalOverride ? ProjectedVelocity.X : CurrentVelocity.X + ProjectedVelocity.X;
			float VelY = bHorrizontalOverride ? ProjectedVelocity.Y : CurrentVelocity.Y + ProjectedVelocity.Y;
			float VelZ = bHorrizontalOverride ? ProjectedVelocity.Z : CurrentVelocity.Z + ProjectedVelocity.Z;

			CurrentVelocity = FVector(VelX, VelY, VelZ);*/

			const FVector RailTangent = GetRailTangentAtPlayerLocation();
			const bool bAdjustTangent = (CurrentVelocity | RailTangent) < 0.0f || (CurrentVelocity.SizeSquared() < UE_SMALL_NUMBER && (RailTangent | PlayerRef->GetHeroMovementComponent()->GetCustomGravityDir()) < 0.0f);

			const float TangentAdjustMultiplier = bAdjustTangent ? -1.0f : 1.0f;
			const FVector AdjustedTangent = RailTangent  * TangentAdjustMultiplier; 
			const float SlopeAcceleration = GetSlopeAcceleration(AdjustedTangent);
			float NewRailSpeed = ProjectedVelocity.Length() + (SlopeAcceleration * DeltaTime);

			if (bForceForwardOnly)
			{
				NewRailSpeed = FMath::Max(NewRailSpeed, MinRailSpeed);
			}

			CurrentVelocity = ProjectedVelocity.GetSafeNormal() * NewRailSpeed;
		}
	}
}

// Called when the game starts or when spawned
void ARailBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARailBase::UpdateTransformOnRail()
{
	if (PlayerRef)
	{
		const FVector AttachLocation = GetClosestPointOnRail();
		const FQuat RailOrientation = GetOrientationOnRail();
		PlayerRef->SetActorLocationAndRotation(AttachLocation, RailOrientation);
		PlayerRef->GetHeroMovementComponent()->SetCurrentSurfaceNormal(RailSpline->FindUpVectorClosestToWorldLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World));
	}	
}

FVector ARailBase::GetRailTangentAtPlayerLocation() const
{
	FVector Tangent = FVector::ZeroVector;
	if (PlayerRef && RailSpline)
	{
		const float DistanceOnRail = RailSpline->GetDistanceAlongSplineAtLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World);
		Tangent = RailSpline->GetDirectionAtDistanceAlongSpline(DistanceOnRail, ESplineCoordinateSpace::World).GetSafeNormal();
	}

	return Tangent;
}

float ARailBase::GetSlopeAcceleration(const FVector& Tangent) const
{
	float Accel = 0.0f;

	if (PlayerRef && RailSpline)
	{
		const UHeroMovementComponent* MovementComp = PlayerRef->GetHeroMovementComponent();
		const FVector Gravity = FVector(0.0f, 0.0f, MovementComp->GetGravityZ());

		Accel = (Tangent | Gravity) * SlopeAccelerationMultiplier;
	}

	return Accel;
}

// Called every frame
void ARailBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(PlayerRef && PlayerRef->bOnRail)
	{
		if(!RailSpline->IsClosedLoop())
		{
			const float TotalRailLength = RailSpline->GetSplineLength();
			const float CurrentDistanceAlongRail = RailSpline->GetDistanceAlongSplineAtLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World);
			const bool bReachedEnd = (CurrentDistanceAlongRail / TotalRailLength >= 1.0f) || (CurrentDistanceAlongRail / TotalRailLength <= 0.0f);
			if (bReachedEnd)
			{
				PlayerRef->OnRailEnd(this);
				return;
			}
		}
		

		UpdateTransformOnRail();
		UpdatePlayerVelocity(DeltaTime, GetVelocityProjectedOnRail(), true, true);
	}

#if WITH_EDITOR
	VisualizeDebug();
#endif
}

#if WITH_EDITOR
void ARailBase::VisualizeDebug()
{
	if (CVarDebugRails.GetValueOnGameThread() != 0)
	{
		AHeroCharacter* Character = Cast<AHeroCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		DrawDebugSphere(GetWorld(), GetClosestPointOnRail(), 10.0f, 12, FColor::Blue);

		float HeightOffset = 0.f;
		const float OffsetPerElement = 10.0f;
		const FVector TopOfCapsule = Character->GetActorLocation() + FVector(0.f, 0.f, PlayerRef->GetSimpleCollisionHalfHeight());

		if (Character->bOnRail)
		{
			FColor DebugColor = FColor::Emerald;
			FVector DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
			FString DebugText = FString::Printf(TEXT("Move Direction On Rail: %f"), GetDirectionOnRail());
			DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

			DebugColor = FColor::Orange;
			DrawDebugDirectionalArrow(GetWorld(), Character->GetActorLocation(), Character->GetActorLocation() + GetVelocityProjectedOnRail().GetSafeNormal() * 75.0f,
				1.f, DebugColor, false, -1.f, (uint8)'\000', 1.f);

			DebugColor = FColor::Yellow;
			HeightOffset += OffsetPerElement;
			DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
			DebugText = FString::Printf(TEXT("Rail Slope Acceleration: %f"), GetSlopeAcceleration(GetRailTangentAtPlayerLocation()));
			DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);

			HeightOffset += OffsetPerElement;
			const float TotalRailLength = RailSpline->GetSplineLength();
			const float CurrentDistanceAlongRail = RailSpline->GetDistanceAlongSplineAtLocation(PlayerRef->GetActorLocation(), ESplineCoordinateSpace::World);

			DebugColor = FColor::Cyan;
			DebugLocation = TopOfCapsule + FVector(0.f, 0.f, HeightOffset);
			DebugText = FString::Printf(TEXT("Rail Progress: %f"), CurrentDistanceAlongRail/ TotalRailLength);
			DrawDebugString(GetWorld(), DebugLocation, DebugText, nullptr, DebugColor, 0.f, true);
		}
	}
}
#endif
