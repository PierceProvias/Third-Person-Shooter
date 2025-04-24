// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PickupWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class BLASTER_API UPickupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UImage> WeaponImage;

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> WeaponName;
};
