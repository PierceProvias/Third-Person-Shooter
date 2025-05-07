// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

UCLASS()
class BLASTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShieldPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
) override;

private:
	UPROPERTY(EditAnywhere, Category = "Health")
	float ShieldReplenishAmount = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float ShieldReplenishTime = 5.f;	// Health will increase gradually
	
};
