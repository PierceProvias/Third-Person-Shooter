// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingActor.generated.h"

class UBoxComponent;
class UInterpToMovementComponent;

UCLASS()
class BLASTER_API AMovingActor : public AActor
{
	GENERATED_BODY()

public:
	AMovingActor();
	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta = (ExposeOnSpawn = "true", MakeEditWidget = "true"))
	TArray<FVector> ThePath;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxCollider;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UInterpToMovementComponent> InterpMovementComponent;
	
};
