// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"

#include "../HUD/BlasterHUD.h"
#include "../HUD/CharacterOverlay.h"
#include "../Characters/BlasterCharacter.h"
#include "../GameModes/BlasterGameMode.h"
#include "../HUD/Announcement.h"
#include "../GameStates/BlasterGameState.h"
#include "../PlayerState/BlasterPlayerState.h"
#include "../BlasterCharacterComponents/CombatComponent.h"
#include "../Weapons/Weapon.h"

#define RENDER_OPACITY_FULL 1.0f

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ServerCheckMatchState();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	
	ServerCheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		GameMode = GameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : GameMode;
		if (GameMode)
		{
			SecondsLeft = FMath::CeilToInt(GameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		double CurrentTime = GetWorld()->GetTimeSeconds();
		ServerRequestServerTime(CurrentTime);
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup_Time, float Match_Time, float Cooldown_Time, float StartingTime)
{
	WarmupTime			= Warmup_Time;
	MatchTime			= Match_Time;
	CooldownTime		= Cooldown_Time;
	LevelStartingTime	= StartingTime;
	MatchState			= StateOfMatch;
	
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	if (ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		WarmupTime			= BlasterGameMode->WarmupTime;
		MatchTime			= BlasterGameMode->MatchTime;
		CooldownTime		= BlasterGameMode->CooldownTime;
		LevelStartingTime	= BlasterGameMode->LevelStartingTime;
		MatchState			= BlasterGameMode->GetMatchState();
		
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	/*
	
	if (MatchState == MatchState::WaitingToStart)
	{
		if (BlasterHUD && BlasterHUD->Annoucement)
		{
			BlasterHUD->Annoucement->SetVisibility(ESlateVisibility::Visible);
		}
	}
	*/
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
	
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	if (BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = BlasterHUD->Annoucement && 
			BlasterHUD->Annoucement->AnnouncementText && 
			BlasterHUD->Annoucement->InfoText;

		if (bHUDValid)
		{
			BlasterHUD->Annoucement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In: ");
			BlasterHUD->Annoucement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

			if (BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					InfoTextString = FString("You are the winner");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto& TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				BlasterHUD->Annoucement->InfoText->SetText(FText::FromString(InfoTextString));
			}

			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(LookOnlyMappingContext, 0);
			}
		}
	}
}


void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->Annoucement)
	{
		BlasterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->Annoucement->SetVisibility(ESlateVisibility::Hidden);
	}	
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}


void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmountText;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmountText->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDDeaths(int32 Deaths)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DeathsAmountText;

	if (bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		BlasterHUD->CharacterOverlay->DeathsAmountText->SetText(FText::FromString(DeathsText));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmountText;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmountText->SetText(FText::FromString(AmmoText));
		BlasterHUD->CharacterOverlay->WeaponAmmoAmountText->SetRenderOpacity(RENDER_OPACITY_FULL);
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmountText;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmountText->SetText(FText::FromString(AmmoText));
		BlasterHUD->CharacterOverlay->CarriedAmmoAmountText->SetRenderOpacity(RENDER_OPACITY_FULL);
	}
}

void ABlasterPlayerController::SetHUDCarriedWeaponTexture(UTexture2D* CurrentWeaponTexture)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponImage;
	
	if (bHUDValid)
	{
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
		{
			CurrentWeaponTexture = BlasterCharacter->GetEquippedWeapon()->GetWeaponTexture2D();
			BlasterHUD->CharacterOverlay->WeaponImage->SetBrushFromTexture(CurrentWeaponTexture);
			BlasterHUD->CharacterOverlay->WeaponImage->SetRenderOpacity(RENDER_OPACITY_FULL);
		}			
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
		if (Minutes < 1 && Seconds <= 30)
		{
			if (Seconds % 2 == 0)
			{
				BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
			}
			else
			{
				BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
			}
		}
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Annoucement &&
		BlasterHUD->Annoucement->WarmupTimerText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Annoucement->WarmupTimerText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Annoucement->WarmupTimerText->SetText(FText::FromString(CountdownText));
		if (Minutes < 1 && Seconds == 10)
		{
			UGameplayStatics::PlaySound2D(this, WarmupCountdownSound);
			BlasterHUD->Annoucement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
		}
		if (CountdownTime <= 10.f && CountdownTime > 1.f)
		{
			UGameplayStatics::PlaySound2D(this, WarmupCountdownSound);
			if (Seconds % 2 == 0)
			{
				BlasterHUD->Annoucement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
			}
			else
			{
				BlasterHUD->Annoucement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
			}
		} 
		else if (CountdownTime <= 1.f)
		{
			UGameplayStatics::PlaySound2D(this, WarmupCountdownSoundEnd);
			if (Seconds % 2 == 0)
			{
				BlasterHUD->Annoucement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
			}
			else
			{
				BlasterHUD->Annoucement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
			}
		}
	}
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
		else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}


