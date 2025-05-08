// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackerCam.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/RadialSlider.h"
#include "GameFramework/PlayerState.h"

void UAttackerCam::SetDisplayText (FString TextToDisplay)
{
	if (AttackerName.IsValid())
	{
		AttackerName.Get()->SetText(FText::FromString(TextToDisplay));
	}
}

void UAttackerCam::ShowPlayerName(APawn* InPawn)
{
	if (APlayerState* PlayerState = InPawn->GetPlayerState())
	{
		FString PlayerName = PlayerState->GetPlayerName();
		SetDisplayText(PlayerName);
	}
}

void UAttackerCam::NativeConstruct()
{
	Super::NativeConstruct();

	
	
}

void UAttackerCam::NativePreConstruct()
{
	Super::NativePreConstruct();

	RadialProgressBarInstance = UMaterialInstanceDynamic::Create(
		RadialProgressBarMaterial,
	this
		);

	RespawnProgressBarImage->SetBrushFromMaterial(RadialProgressBarInstance);
	RadialProgressBarInstance->SetScalarParameterValue(TEXT("Percent"), GetPercent());
	
}
