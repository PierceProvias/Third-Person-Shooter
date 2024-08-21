// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"


#define MIN_DAMAGE		10.f
#define INNER_RADIUS	200.f
#define OUTER_RADIUS	500.f
#define DAMAGE_FALLOFF	1.f

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(GetRootComponent());
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (TrailSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Returns pawn that owns this rocket
	if (APawn* FiringPawn = GetInstigator())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,						// World context object
				Damage,						// Base Damage
				MIN_DAMAGE,
				GetActorLocation(),			// Origin
				INNER_RADIUS,
				OUTER_RADIUS,
				DAMAGE_FALLOFF,
				UDamageType::StaticClass(),	// Damage type class
				TArray<AActor*>(),			// Ignore Actors
				this,						// Damage cause
				FiringController			// Instigator controller
			);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

