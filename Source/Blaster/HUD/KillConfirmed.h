#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillConfirmed.generated.h"

class UImage;

UCLASS()
class BLASTER_API UKillConfirmed : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TWeakObjectPtr<UImage> KillConfirmedImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))	
	TWeakObjectPtr<UWidgetAnimation> KillConfirmedAnimation;
};
