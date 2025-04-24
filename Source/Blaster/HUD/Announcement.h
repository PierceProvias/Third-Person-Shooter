// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

UCLASS()
class BLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> WarmupTimerText;

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UTextBlock> InfoText;
};
