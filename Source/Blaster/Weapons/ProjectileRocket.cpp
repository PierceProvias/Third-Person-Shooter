// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

#define MIN_DAMAGE			10.f
#define INNER_RADIUS		200.f
#define OUTER_RADIUS		500.f
#define DAMAGE_FALLOFF		1.f
#define VOLUME_MULTIPLIER	1.f
#define PITCH_MULTIPLIER	1.f

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(GetRootComponent());
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
	
	RocketMovementComponent->InitialSpeed = 1500.f;
	RocketMovementComponent->MaxSpeed = 1500.f;
	RocketMovementComponent->ProjectileGravityScale = 0.f;
	SetLifeSpan(10.f);
}

void AProjectileRocket::Destroyed()
{

}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}
	if (TrailSystem)
	{
		
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			VOLUME_MULTIPLIER,
			PITCH_MULTIPLIER,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner()) return;
	// Returns pawn that owns this rocket
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
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
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime
	);

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

