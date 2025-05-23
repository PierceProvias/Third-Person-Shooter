// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

#include "../PlayerController/BlasterPlayerController.h"
#include "../Characters/BlasterCharacter.h"
#include "../Weapons/Weapon.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Deaths);
}

// Client
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if (BlasterCharacter.IsValid())
	{
		BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
		if (BlasterController.IsValid())
		{
			BlasterController->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::OnRep_Deaths()
{
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if (BlasterCharacter.IsValid())
	{
		BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
		if (BlasterController.IsValid())
		{
			BlasterController->SetHUDDeaths(Deaths);
			// BlasterController->SetViewTargetWithBlend(this->GetPawn(), 1.f);
			// BlasterController->SetAttackerCam(BlasterController->);
		}
	}
}

// Server
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if (BlasterCharacter.IsValid())
	{
		BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
		if (BlasterController.IsValid())
		{
			BlasterController->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if (BlasterCharacter.IsValid())
	{
		BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
		if (BlasterController.IsValid())
		{
			BlasterController->SetHUDDeaths(Deaths);
		}
	}
}

