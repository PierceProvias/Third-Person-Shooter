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

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget, bool DebugEnabled = false);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bEnableDebug = true;

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

	/*
	* Trace end with scatter
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

};
