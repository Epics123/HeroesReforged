// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldToScreenWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"

void UWorldToScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UWorldToScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector WorldLocation = TargetLocation + Offset;

	FVector2D OutScreenPos;
	const bool bOnScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), WorldLocation, OutScreenPos, true);

	if(bOnScreen)
	{
		const FVector2D Size = GetDesiredSize();
		const FVector2D CenteredTranslation(OutScreenPos.X - Size.X * 0.5f, OutScreenPos.Y - Size.Y * 0.5f);
		SetRenderTranslation(CenteredTranslation);
	}
}
