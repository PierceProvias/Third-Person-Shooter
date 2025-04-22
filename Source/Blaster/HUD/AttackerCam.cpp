// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackerCam.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UAttackerCam::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
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
