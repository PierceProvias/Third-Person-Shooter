// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "DamageCamera.generated.h"

class UPerlinNoiseCameraShakePattern; 

UCLASS()
class BLASTER_API UDamageCamera : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UDamageCamera();

	UPROPERTY(EditAnywhere, Category = "Camera Shake")
	TObjectPtr<UPerlinNoiseCameraShakePattern> DamageShakePattern;
};
