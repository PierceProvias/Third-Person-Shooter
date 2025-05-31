#include "BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

#include "../PlayerController/BlasterPlayerController.h"
#include "../Characters/BlasterCharacter.h"
#include "../PlayerState/BlasterPlayerState.h" 
#include "../GameStates/BlasterGameState.h"
#include "GameFramework/PlayerController.h"

#define VICTIM_TARGET_BLEND 0.5f

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It))
		{
			BlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - (LevelStartingTime + GetWorld()->GetTimeSeconds());
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
	
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	
	ABlasterPlayerState* VictimPlayerState = VictimController? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1.f);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
		if (HasAuthority())
		{
			SwitchToAttackerCamera(VictimController, AttackerController);
		}
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();	// Detach from controller in order to capture another actor
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
		if (ABlasterPlayerController* ElimmedBlasterPlayerController = Cast<ABlasterPlayerController>(ElimmedController))
		{
			ElimmedBlasterPlayerController->ResetCharacterOverlay();
		}
	}
}

void ABlasterGameMode::SwitchToAttackerCamera(ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	
	if (AttackerController && AttackerPlayerState && VictimController && VictimPlayerState)
	{
		VictimController->SetViewTargetWithBlend(AttackerPlayerState->GetPawn(), VICTIM_TARGET_BLEND);
		VictimController->SetAttackerCam(AttackerController);
	}
}

