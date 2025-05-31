// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;

protected:

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter | Debug")
	bool bEnableScatterDebug = false;

	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundCue> HitSound;
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:

	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UParticleSystem> BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UParticleSystem> MuzzleFlash;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundCue> FireSound;
};
