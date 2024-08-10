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
	TObjectPtr<ABlasterHUD> BlasterHUD;
	void SetHUDHealth(float Health, float MaxHealth);
	void OnPossess(APawn* InPawn) override;
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);

protected:
	virtual void BeginPlay() override;
};
