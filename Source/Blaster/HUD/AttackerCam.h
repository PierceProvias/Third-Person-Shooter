// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttackerCam.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class BLASTER_API UAttackerCam : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AttackerProfileImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayText;

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn);
};
