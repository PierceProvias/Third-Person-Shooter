// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttackerCam.generated.h"


class UTextBlock;
class UImage;
class URadialSlider;

UCLASS()
class BLASTER_API UAttackerCam : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UImage> AttackerProfileImage;

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<URadialSlider> RespawnProgressBar;

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> AttackerName;


	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> RespawnTime;

	UFUNCTION(BlueprintCallable)
	void StartSliderAnimation(float TargetValue);

	UFUNCTION(BlueprintCallable)
	void StartRespawnText(float RespawnTimeText);

	UFUNCTION()
	void UpdateSliderValue();

	UFUNCTION()
	void UpdateRespawnText();
	
	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn);
	
protected:
	FTimerHandle SlideTimerHandle;
	FTimerHandle RespawnTimerHandle;
	float SlideDuration = 3.0f;
	float RespawnDuration = 3.0f;
	float SlideStartTime;
	float RespawnStartTime;

	float StartValue;
	float EndValue;
	
};
