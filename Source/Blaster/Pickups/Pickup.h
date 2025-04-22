// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundCue;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable)
	float TransformedSin();
protected:
	
	virtual void BeginPlay() override;
	bool bShouldTransform = true;
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinusoids")
	float Amplitude = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinusoids")
	float TimeConstant = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sinusoids")
	float RunningTime;
private:

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	TObjectPtr<USoundCue> PickupSound;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> PickupMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Niagara")
	TObjectPtr<UNiagaraComponent> PickupEffectComponent;
    	
	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> PickupEffect;
	
public:	
	
	FORCEINLINE USphereComponent* GetOverlapSphere() const { return OverlapSphere; }
	FORCEINLINE UNiagaraComponent* GetPickupEffectComponent() const { return PickupEffectComponent; }
	FORCEINLINE UNiagaraSystem* GetPickupEffect() const { return PickupEffect; }
};
