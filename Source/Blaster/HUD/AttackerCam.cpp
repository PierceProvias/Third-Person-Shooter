// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackerCam.h"
#include "Components/TextBlock.h"
#include "Components/RadialSlider.h"
#include "GameFramework/PlayerState.h"


void UAttackerCam::UpdateSliderValue()
{
	if (RespawnProgressBar.IsValid())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime - SlideStartTime;

		if (ElapsedTime <= SlideDuration)
		{
			float Alpha = FMath::Clamp(ElapsedTime / SlideDuration, 0.0f, 1.0f);
			float NewValue = FMath::Lerp(StartValue, EndValue, Alpha);
			RespawnProgressBar->SetValue(NewValue);
		}
		else
		{
			RespawnProgressBar->SetValue(StartValue);
			GetWorld()->GetTimerManager().ClearTimer(SlideTimerHandle);
		}
	}
}

void UAttackerCam::UpdateRespawnText()
{
	if (RespawnTime.IsValid())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime -1.f - RespawnStartTime;
		if (ElapsedTime <= RespawnDuration)
		{
			float Alpha = FMath::Clamp(ElapsedTime / RespawnDuration, 0.f, 1.0f);
			float NewValue = FMath::Lerp(EndValue, StartValue, Alpha);
			FString RespawTimeText = FString::Printf(TEXT("%d"), (uint32)NewValue);
			RespawnTime->SetText(FText::FromString(RespawTimeText));
		}
	}
}

void UAttackerCam::StartSliderAnimation(float TargetValue)
{
	if (RespawnProgressBar.IsValid())
	{
		StartValue = RespawnProgressBar->GetValue();
		EndValue = TargetValue;
		SlideStartTime = GetWorld()->GetTimeSeconds();

		GetWorld()->GetTimerManager().SetTimer(
			SlideTimerHandle,
			this,
			&UAttackerCam::UpdateSliderValue,
			GetWorld()->GetDeltaSeconds(),
			true
		);
		RespawnProgressBar->SetStepSize(1.f);
	}
}

void UAttackerCam::StartRespawnText(float TargetValue)
{
	if (RespawnProgressBar.IsValid())
	{
		StartValue = RespawnProgressBar->GetValue();
		EndValue = TargetValue;
		RespawnStartTime = GetWorld()->GetTimeSeconds();

		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&UAttackerCam::UpdateRespawnText,
			GetWorld()->GetDeltaSeconds(),
			true);
	}
}

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