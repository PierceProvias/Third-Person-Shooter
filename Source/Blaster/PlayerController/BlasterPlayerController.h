// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float Health, float MaxHealth);
	void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);

	// Synced with server world clock
	virtual float GetServerTime();

	// Sync with server clock as soon as possible
	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

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

private:
	TObjectPtr<ABlasterHUD> BlasterHUD;
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
};
