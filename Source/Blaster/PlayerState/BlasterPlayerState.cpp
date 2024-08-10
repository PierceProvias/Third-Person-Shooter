// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "../Characters/BlasterCharacter.h"
#include "../PlayerController/BlasterPlayerController.h"

// Client
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
		if (BlasterController)
		{
			BlasterController->SetHUDScore(Score);
		}
	}
}

// Server
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	
	Score += ScoreAmount;
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
		if (BlasterController)
		{
			BlasterController->SetHUDScore(Score);
		}
	}
}
