// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	// Must be a UFUNCTION when callbacks are being binded
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Already replicated
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> ImpactSound;

private:


	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> Tracer;

	TObjectPtr< UParticleSystemComponent> TracerComponent;





};
