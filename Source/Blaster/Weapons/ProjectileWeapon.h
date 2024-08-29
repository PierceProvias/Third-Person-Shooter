// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;
class USound;

UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
	
public: 

	virtual void Fire(const FVector& HitTarget) override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMuzzleFlash();

private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;


	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UParticleSystem> BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Particles")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundCue> FireSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundCue> HitSound;
};
