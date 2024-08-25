// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	virtual void Destroyed() override;

protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	void DestroyTimerFinished();

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> TrailSystem;

	TObjectPtr <UNiagaraComponent> TrailSystemComponent;

	/*
	* Projectile Rocket while in air
	*/

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundCue> ProjectileLoop;

	TObjectPtr<UAudioComponent> ProjectileLoopComponent;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundAttenuation> LoopingSoundAttenuation;

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RocketMesh;

	FTimerHandle DestroyTimer;
	
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

};
