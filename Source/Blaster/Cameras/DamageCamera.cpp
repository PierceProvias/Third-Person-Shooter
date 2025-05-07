// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageCamera.h"
#include "PerlinNoiseCameraShakePattern.h"

UDamageCamera::UDamageCamera()
	: UCameraShakeBase(FObjectInitializer::Get())
{
	bSingleInstance = false;

	DamageShakePattern = CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("DamageShakePattern"));

	DamageShakePattern->X.Amplitude = 10.f;
	DamageShakePattern->X.Frequency = 10.f;
	DamageShakePattern->Y.Amplitude = 10.f;
	DamageShakePattern->Y.Frequency = 10.f;
	DamageShakePattern->Z.Amplitude = 10.f;
	DamageShakePattern->Z.Frequency = 10.f;

	DamageShakePattern->Pitch.Amplitude = 10.f;
	DamageShakePattern->Pitch.Frequency = 10.f;
	DamageShakePattern->Yaw.Amplitude = 10.f;
	DamageShakePattern->Yaw.Frequency = 10.f;
	DamageShakePattern->Roll.Amplitude = 10.f;
	DamageShakePattern->Roll.Frequency = 10.f;

	DamageShakePattern->FOV.Amplitude = 10.f;
	DamageShakePattern->FOV.Frequency = 10.f;

	DamageShakePattern->Duration = 0.25f;

	SetRootShakePattern(DamageShakePattern);
}
