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
	TObjectPtr<ABlasterHUD> BlasterHUD;
	void SetHUDHealth(float Health, float MaxHealth);
	void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

private:
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
