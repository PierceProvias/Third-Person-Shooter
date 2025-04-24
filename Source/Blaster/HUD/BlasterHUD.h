// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UTexture2D;
class UCharacterOverlay;
class UAnnouncement;
class UAttackerCam;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	
	TObjectPtr<UTexture2D> CrosshairsCenter;
	TObjectPtr<UTexture2D> CrosshairsLeft;
	TObjectPtr<UTexture2D> CrosshairsRight;
	TObjectPtr<UTexture2D> CrosshairsTop;
	TObjectPtr<UTexture2D> CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
	
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public: 

	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category = "Annoucements")
	TSubclassOf<UUserWidget> AnnouncementClass;
	
	UPROPERTY(EditAnywhere, Category = "Annoucements")
    TSubclassOf<UUserWidget> AttackerCamClass;
    	
	TObjectPtr<UCharacterOverlay> CharacterOverlay;
	TWeakObjectPtr<UAnnouncement> Announcement;
	TObjectPtr<UAttackerCam> AttackerCam;
	
	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddAttackerCam();
protected:

	virtual void BeginPlay() override;

private:

	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

	
};
