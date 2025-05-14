// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class UAttackerCam;
class UCharacterOverlay;
class ABlasterHUD;
class ABlasterGameMode;
class UInputMappingContext;
class USoundBase;
class ABlasterPlayerController;
class ABlasterCharacter;

UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDPrimaryGrenades(int32 Grenades);
	void SetHUDSecondaryGrenades(int32 Grenades);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDCarriedWeaponTexture(UTexture2D* CurrentWeaponTexture);
	void SetHUDMatchCountdown(float CountdownTime);
	void ResetCharacterOverlay();
	void SetHUDAnnouncementCountdown(float CountdownTime);
	
	void SetAttackerCam(const ABlasterPlayerController* AttackerController);
	void SetHUDRespawmTimer(ABlasterCharacter* ElimmedBlasterCharacter, float RespawnTime);

	// Synced with server world clock
	virtual float GetServerTime();

	// Sync with server clock as soon as possible
	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

	UFUNCTION()
	void OnRep_MatchState();
	void HandleMatchHasStarted();
	void HandleCooldown();

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	/*
	* Sync time between client and server
	*/
	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;	// Difference between client and server time

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;	

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

	// Server RPC to initialize the player con`troller in the right match state
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// Large client RPC which will only happen once (when joining)
	
	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup_Time, float Match_Time, float Cooldown_Time, float StartingTime);

private:
	TWeakObjectPtr<ABlasterHUD> BlasterHUD;
	TWeakObjectPtr<UCharacterOverlay> CharacterOverlay;
	TWeakObjectPtr<UAttackerCam> AttackerCam ;
	TWeakObjectPtr<ABlasterGameMode> GameMode;

	float LevelStartingTime = 0.f;
	float WarmupTime		= 0.f;
	float CooldownTime		= 0.f;
	float MatchTime			= 0.f;	// Player controller should be getting the match time from the game mode

	uint32 CountdownInt = 0;

	FTimerHandle RespawnTimerHandle;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> LookOnlyMappingContext;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> WarmupCountdownSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> WarmupCountdownSoundEnd;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDWeaponAmmo;
	float HUDCarriedAmmo;

	int32 HUDScore				= 0;
	int32 HUDDeaths				= 0;
	int32 HUDPrimaryGrenades	= 0;
	int32 HUDSecondaryGrenades	= 0;

	TObjectPtr<UTexture2D> WeaponTexture;

	bool bInitializeScore				= false;
	bool bInitializeDeaths				= false;
	bool bInitializeHealth				= false;
	bool bInitializeShield				= false;
	bool bInitializeWeaponAmmo			= false;
	bool bInitializeCarriedAmmo			= false;
	bool bInitializeWeaponTexture		= false;
	bool bInitializePrimaryGrenades		= false;
	bool bInitializeSecondaryGrenades	= false;
	bool bInitializeAttackerCam			= false;
	bool bInitializeRespawnTimer		= false;
	
	
};
