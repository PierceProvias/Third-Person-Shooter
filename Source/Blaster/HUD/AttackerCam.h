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
	
	// UPROPERTY(EditDefaultsOnly)
	// TObjectPtr<UMaterial> RadialProgressBarMaterial;
	//
	// UPROPERTY(EditDefaultsOnly)
	// TObjectPtr<UMaterialInstanceDynamic> RadialProgressBarInstance;
	//
	// UPROPERTY(meta = (BindWidget))
	// TWeakObjectPtr<UImage> RespawnProgressBarImage;
	

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> RespawnTime;

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn);


protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
};
