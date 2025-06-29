// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAIController.h"

void AHeroAIController::OnPossess(APawn* InPawn)
{
	const bool bNewPawn = GetPawn() != InPawn;

	// Unpossess current pawn (if any) when current pawn changes
	if (bNewPawn && GetPawn() != nullptr)
	{
		UnPossess();
	}

	if (InPawn == nullptr)
	{
		return;
	}

	if (InPawn->Controller != nullptr)
	{
		UE_CLOG(InPawn->Controller == this, LogController, Warning, TEXT("Asking %s to possess pawn %s more than once; pawn will be restarted! Should call Unpossess first."), *GetNameSafe(this), *GetNameSafe(InPawn));
		InPawn->Controller->UnPossess();
	}

	InPawn->PossessedBy(this);
	SetPawn(InPawn);

	// update rotation to match possessed pawn's rotation
	SetControlRotation(GetPawn()->GetActorRotation());
}
