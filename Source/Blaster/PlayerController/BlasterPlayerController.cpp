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
#include "../Weapons/Weapon.h"
#include "../HUD/AttackerCam.h"
#include "Components/RadialSlider.h"

#define RENDER_OPACITY_FULL 1.0f
#define RENDER_OPACITY_EMPTY 0.f
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
	PollInit();
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
		if (GameMode.IsValid())
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

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD.IsValid() && BlasterHUD->CharacterOverlay)
		{
			if(CharacterOverlay.IsValid())
			{
				if (bInitializeScore)					SetHUDScore(HUDScore);
				if (bInitializeDeaths)					SetHUDDeaths(HUDDeaths);
				if (bInitializeHealth)					SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield)					SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeCarriedAmmo)				SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo)				SetHUDWeaponAmmo(HUDWeaponAmmo);
				
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter && BlasterCharacter->GetCombatComponent())
				{
					if (bInitializePrimaryGrenades)		SetHUDPrimaryGrenades(HUDPrimaryGrenades);
					if (bInitializeSecondaryGrenades)	SetHUDSecondaryGrenades(HUDSecondaryGrenades);
				}
			}
			if (BlasterHUD->Announcement.IsValid())
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(GetPawn());
				if (BlasterCharacter)
				{
					if (BlasterCharacter->IsElimmed())
					{
						if (bInitializeAttackerCam && BlasterPlayerController)	SetAttackerCam(BlasterPlayerController); 
					}
				}
			}
		}
	}
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

void ABlasterPlayerController::RespawnTimerFinished()
{
	if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
	{
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(this->GetPawn()))
		{
			BlasterGameMode->RequestRespawn(BlasterCharacter, this);
		}
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
	if (BlasterHUD.IsValid() && MatchState == MatchState::WaitingToStart)
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
		if (BlasterHUD.IsValid() && MatchState == MatchState::WaitingToStart)
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
		if (BlasterHUD && BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
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
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD.IsValid())
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = BlasterHUD->Announcement.IsValid() && 
			BlasterHUD->Announcement->AnnouncementText.IsValid() && 
			BlasterHUD->Announcement->InfoText.IsValid();

		if (bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In: ");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
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
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
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

	if (BlasterHUD.IsValid() && BlasterHUD->CharacterOverlay && BlasterHUD->Announcement.IsValid())
	{
		BlasterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
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
	bool bHUDValid = BlasterHUD.IsValid() &&
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
	else
	{
		// Store values in case SetHUDHealth is called before the CharacterOverlay is set
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmountText;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmountText->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDeaths(int32 Deaths)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay&&
		BlasterHUD->CharacterOverlay->DeathsAmountText;

	if (bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		BlasterHUD->CharacterOverlay->DeathsAmountText->SetText(FText::FromString(DeathsText));
	}
	else
	{
		bInitializeDeaths = true;
		HUDDeaths = Deaths;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmountText;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmountText->SetText(FText::FromString(AmmoText));
		BlasterHUD->CharacterOverlay->WeaponAmmoAmountText->SetRenderOpacity(RENDER_OPACITY_FULL);
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDPrimaryGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->PrimaryGrenadeCountText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->PrimaryGrenadeCountText->SetText(FText::FromString(GrenadesText));
		BlasterHUD->CharacterOverlay->PrimaryGrenadeCountText->SetRenderOpacity(RENDER_OPACITY_FULL);
	}
	else
	{
		bInitializePrimaryGrenades = true;
		HUDPrimaryGrenades = Grenades;
	}
}

void ABlasterPlayerController::SetHUDSecondaryGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->SecondaryGrenadeCountText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->SecondaryGrenadeCountText->SetText(FText::FromString(GrenadesText));
		BlasterHUD->CharacterOverlay->SecondaryGrenadeCountText->SetRenderOpacity(RENDER_OPACITY_FULL);
	}
	else
	{
		bInitializeSecondaryGrenades = true;
		HUDSecondaryGrenades = Grenades;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmountText;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmountText->SetText(FText::FromString(AmmoText));
		BlasterHUD->CharacterOverlay->CarriedAmmoAmountText->SetRenderOpacity(RENDER_OPACITY_FULL);
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedWeaponTexture(UTexture2D* CurrentWeaponTexture)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponImage;
	
	if (bHUDValid)
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
		if (BlasterCharacter)
		{
			CurrentWeaponTexture = BlasterCharacter->GetEquippedWeapon()->GetWeaponTexture2D();
			BlasterHUD->CharacterOverlay->WeaponImage->SetBrushFromTexture(CurrentWeaponTexture);
			BlasterHUD->CharacterOverlay->WeaponImage->SetRenderOpacity(RENDER_OPACITY_FULL);
		}
		// TODO: When player respawns no weapon texture should be shown
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
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

void ABlasterPlayerController::ResetCharacterOverlay()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->AttackerCam.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->AttackerCam.IsValid() &&
		BlasterHUD->AttackerCam.IsValid();
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->AttackerCam->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->Announcement.IsValid() &&
		BlasterHUD->Announcement->WarmupTimerText.IsValid();

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTimerText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTimerText->SetText(FText::FromString(CountdownText));
		if (Minutes < 1 && Seconds == 10)
		{
			UGameplayStatics::PlaySound2D(this, WarmupCountdownSound);
			BlasterHUD->Announcement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
		}
		if (CountdownTime <= 10.f && CountdownTime > 1.f)
		{
			UGameplayStatics::PlaySound2D(this, WarmupCountdownSound);
			if (Seconds % 2 == 0)
			{
				BlasterHUD->Announcement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
			}
			else
			{
				BlasterHUD->Announcement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
			}
		} 
		else if (CountdownTime <= 1.f)
		{
			UGameplayStatics::PlaySound2D(this, WarmupCountdownSoundEnd);
			if (Seconds % 2 == 0)
			{
				BlasterHUD->Announcement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 0, 0, 1));
			}
			else
			{
				BlasterHUD->Announcement->WarmupTimerText->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
			}
		}
	}
}

void ABlasterPlayerController::SetAttackerCam(ABlasterPlayerController* AttackerController)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->AttackerCam.IsValid() &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->AttackerCam->AttackerProfileImage.IsValid() &&
		BlasterHUD->AttackerCam->AttackerName.IsValid();
	
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Collapsed);
		BlasterHUD->AttackerCam->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->AttackerCam->ShowPlayerName(AttackerController->GetPawn());
		if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(AttackerController->GetPawn()))
		{
			UE_LOG(LogTemp, Warning, TEXT("SetATtackerCam"));
			SetHUDRespawmTimer(BlasterCharacter, BlasterCharacter->GetElimDelay());
		}
	}
	else
	{
		bInitializeAttackerCam = true;
	}
}

void ABlasterPlayerController::SetHUDRespawmTimer(ABlasterCharacter* BlasterCharacter, float RespawmTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD.IsValid() &&
		BlasterHUD->AttackerCam.IsValid() &&
		BlasterHUD->AttackerCam->RespawnProgressBar.IsValid() &&
		BlasterHUD->AttackerCam->RespawnTime.IsValid();
	
	if (bHUDValid)
	{
		float RespawnDelay = BlasterCharacter->GetElimDelay();
		BlasterHUD->AttackerCam->RespawnProgressBar->SetValue(RespawnDelay);
		//BlasterHUD->AttackerCam->RespawnProgressBar->S
		//BlasterHUD->AttackerCam->RespawnProgressBar->ValueDelegate -= {[RespawmTime](float DeltaTime){}};
		//UE_LOG(LogTemp, Warning, TEXT("Respawn Timer is: %f, SeverTime: %f"), RespawnTimer, GetServerTime());
		GetWorldTimerManager().SetTimer(
			RespawnTimer,
			this,
			&ABlasterPlayerController::RespawnTimerFinished,
			RespawnDelay
			);
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


