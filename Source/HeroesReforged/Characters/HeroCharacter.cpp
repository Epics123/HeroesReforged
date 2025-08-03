// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"
#include "../Components/HeroMovementComponent.h"
#include "../Components/HeroCameraComponent.h"
#include "../Components/HeroAIComponent.h"
#include "../Data/PlayerInputData.h"
#include "../HeroesReforgedGameMode.h"
#include "../HeroManager.h"
#include "HeroAIController.h"

#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/InputDelegateBinding.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeroCharacter, Error, All);

// Sets default values
AHeroCharacter::AHeroCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UHeroMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create camera pivot
	CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPivot"));
	CameraPivot->SetupAttachment(RootComponent);
	CameraPivot->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CameraPivot);
	CameraBoom->TargetArmLength = 500.0f; // The camera follows at this distance behind the character
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, -100);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	JumpballPivot = CreateDefaultSubobject<USceneComponent>(TEXT("JumpballPivot"));
	JumpballPivot->SetupAttachment(RootComponent);

	JumpballMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("JumpballMesh"));
	JumpballMesh->SetupAttachment(JumpballPivot);

	JumpballFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("JumpballFX"));
	JumpballFX->SetupAttachment(JumpballMesh);

	JumpballFX->SetPaused(true);
	JumpballFX->SetVisibility(false);
	JumpballMesh->SetVisibility(false, true);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UHeroCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	HeroAIComponent = CreateDefaultSubobject<UHeroAIComponent>(TEXT("HeroAIComponent"));

	if(HeroAIComponent)
	{
		RightAITarget = CreateDefaultSubobject<USceneComponent>(TEXT("RightAITarget"));
		RightAITarget->SetupAttachment(RootComponent);
		LeftAITarget = CreateDefaultSubobject<USceneComponent>(TEXT("LeftAITarget"));
		LeftAITarget->SetupAttachment(RootComponent);
	}
}

UHeroMovementComponent* AHeroCharacter::GetHeroMovementComponent() const
{
	return Cast<UHeroMovementComponent>(GetMovementComponent());
}

bool AHeroCharacter::IsMoveInputBlocked() const
{
	return false; // TODO: Actually check if input is blocked
}

void AHeroCharacter::SetAIComponentEnabled(bool bEnable)
{
	if(!HeroAIComponent)
	{
		return;
	}

	EHeroAIComponentState AIState = bEnable ? EHeroAIComponentState::Active : EHeroAIComponentState::Inactive;

	HeroAIComponent->SetAIComponentState(AIState);
}

void AHeroCharacter::ShowJumpball()
{
	if (JumpballMesh && JumpballFX && !JumpballMesh->IsVisible())
	{
		GetMesh()->SetVisibility(false);

		JumpballFX->SetPaused(false);
		JumpballMesh->SetVisibility(true, true);
	}
}

void AHeroCharacter::HideJumpball()
{
	if(JumpballMesh && JumpballFX && JumpballMesh->IsVisible())
	{
		GetMesh()->SetVisibility(true);

		JumpballFX->SetPaused(true);
		JumpballMesh->SetVisibility(false, true);
	}
}

float AHeroCharacter::GetJumpballPitchDuringJump(float ApexProximity, float MinDownwardRotation, float MaxUpwardRotation)
{
	UHeroMovementComponent* MovementComponent = GetHeroMovementComponent();
	if(MovementComponent)
	{
		float TargetRotationAngle = 0.0f;

		const float VerticalVelocity = MovementComponent->RotateWorldToGravity(MovementComponent->Velocity).Z;
		if (VerticalVelocity > 0.0f)
		{
			TargetRotationAngle = FMath::Lerp(MaxUpwardRotation, 0.0f, ApexProximity);
		}
		else
		{
			TargetRotationAngle = FMath::Lerp(0.0f, MinDownwardRotation, 1.0f - ApexProximity);
		}

		return TargetRotationAngle;
	}

	return 0.0f;
}

void AHeroCharacter::StartSecondaryJumpAction_Implementation()
{

}

void AHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save AI controllers into a map

	AIController = GetWorld()->SpawnActor<AHeroAIController>(AHeroAIController::StaticClass(), GetActorLocation(), GetActorRotation(), SpawnInfo);
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (InputData)
			{
				UInputMappingContext* DefaultMappingContext = InputData->DefaultMappingContext;
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}

		}
	}
}

void AHeroCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);

	if (PlayerController != nullptr && !IsMoveInputBlocked())
	{
		const UHeroMovementComponent* MovementComp = GetHeroMovementComponent();

		const FVector SurfaceNormal = MovementComp->GetCurrentSurfaceNormal();
		const bool bUseCharacterVectors = (((SurfaceNormal | FVector(0, 0, 1)) < MovementComp->MaxCameraVectorAngle)) || MovementComp->bForceCharacterVectors;

		if (bUseCharacterVectors)
		{
			const FVector VelocityDir = MovementComp->GetLastUpdateVelocity().GetSafeNormal();

			// get forward vector
			const FVector ForwardDirection = VelocityDir.IsNearlyZero(10.0f) ? GetActorForwardVector() : VelocityDir;

			// get right vector 
			const FVector RightDirection = (MovementComp->GetCurrentSurfaceNormal() ^ ForwardDirection).GetSafeNormal();

			//add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
		else
		{
			const FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			FVector CameraForward = CameraRotation.Vector();
			FVector CameraRight = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);

			CameraForward = (CameraForward - (CameraForward | SurfaceNormal) * SurfaceNormal).GetSafeNormal();
			CameraRight = (CameraRight - (CameraRight | SurfaceNormal) * SurfaceNormal).GetSafeNormal();

			const FVector MoveDirection = ((CameraForward * MovementVector.Y) + (CameraRight * MovementVector.X)).GetSafeNormal();
			AddMovementInput(MoveDirection, 1.0f);
		}

		AHeroesReforgedGameMode* GameMode = Cast<AHeroesReforgedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode && GameMode->HeroManager->IsActiveHero(this))
		{
			GameMode->HeroManager->AddMovementInput();
		}
	}
}

void AHeroCharacter::MoveReleased()
{

}

void AHeroCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHeroCharacter::Jump()
{
	GetHeroMovementComponent()->bNotifyApex = true;
	Super::Jump();

	AHeroesReforgedGameMode* GameMode = Cast<AHeroesReforgedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode && GameMode->HeroManager->IsActiveHero(this))
	{
		OnHeroJumped.Broadcast();
	}
}

void AHeroCharacter::StopJumping()
{
	Super::StopJumping();

	AHeroesReforgedGameMode* GameMode = Cast<AHeroesReforgedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode && GameMode->HeroManager->IsActiveHero(this))
	{
		OnHeroStopJump.Broadcast();
	}
}

void AHeroCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	HideJumpball();
	GetHeroMovementComponent()->bNotifyApex = false;
	GetHeroMovementComponent()->ResetAirDeceleration();
}

void AHeroCharacter::CheckSecondaryJumpAction()
{
	if (GetHeroMovementComponent()->IsFalling())
	{
		StartSecondaryJumpAction();
	}
}

void AHeroCharacter::SwapLeft()
{
	SwapHeroInternal(-1);
}

void AHeroCharacter::SwapRight()
{
	SwapHeroInternal(1);
}

void AHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AHeroesReforgedGameMode* GameMode = Cast<AHeroesReforgedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		if(Cast<AHeroAIController>(NewController))
		{
			GetHeroMovementComponent()->Velocity = GameMode->HeroManager->GetSwappedHeroVelocity();
		}
		// Possess previous hero with AI controller and maintain the old character's velocity
		else
		{
			GameMode->HeroManager->AIPossessPreviousHero();
			GetHeroMovementComponent()->Velocity = GameMode->HeroManager->GetTargetHeroVelocity();
		}
	}
}

void AHeroCharacter::PawnClientRestart()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC && PC->IsLocalController())
	{
		// Handle camera possession
		if (PC->bAutoManageActiveCameraTarget)
		{
			PC->AutoManageActiveCameraTarget(this);
		}

		// Set up player input component, if there isn't one already.
		if (InputComponent == nullptr)
		{
			InputComponent = CreatePlayerInputComponent();
			if (InputComponent)
			{
				SetupPlayerInputComponent(InputComponent);
				InputComponent->RegisterComponent();
				if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
				{
					InputComponent->bBlockInput = bBlockInput;
					UInputDelegateBinding::BindInputDelegatesWithSubojects(this, InputComponent);
				}
			}
		}
	}
}

void AHeroCharacter::SwapHeroInternal(int32 Direction)
{
	AHeroesReforgedGameMode* GameMode = Cast<AHeroesReforgedGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->HeroManager->RotateActiveHero(Direction);
		GetWorld()->GetFirstPlayerController()->Possess(GameMode->HeroManager->ActiveHero);
	}
}

// Called every frame
void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputData)
		{
			// Jumping
			EnhancedInputComponent->BindAction(InputData->JumpAction, ETriggerEvent::Started, this, &AHeroCharacter::Jump);
			EnhancedInputComponent->BindAction(InputData->JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
			EnhancedInputComponent->BindAction(InputData->JumpAction, ETriggerEvent::Started, this, &AHeroCharacter::CheckSecondaryJumpAction);

			// Moving
			EnhancedInputComponent->BindAction(InputData->MoveAction, ETriggerEvent::Triggered, this, &AHeroCharacter::Move);
			EnhancedInputComponent->BindAction(InputData->MoveAction, ETriggerEvent::Completed, this, &AHeroCharacter::MoveReleased);

			// Looking
			EnhancedInputComponent->BindAction(InputData->LookAction, ETriggerEvent::Triggered, this, &AHeroCharacter::Look);

			EnhancedInputComponent->BindAction(InputData->SwapLeftAction, ETriggerEvent::Triggered, this, &AHeroCharacter::SwapLeft);
			EnhancedInputComponent->BindAction(InputData->SwapRightAction, ETriggerEvent::Triggered, this, &AHeroCharacter::SwapRight);
		}
	}
	else
	{
		UE_LOG(LogHeroCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

}

