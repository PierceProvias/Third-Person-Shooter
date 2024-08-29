// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;

	//BlasterCharacter&& HasAuthority() && InstigatorController
	//APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//AController* InstigatorController = InstigatorPawn->GetController();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash")))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorController)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = OwnerPawn;
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
				);

			}
		}
		if (UWorld* World = GetWorld())
		{
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransform
				);

			}
			if (MuzzleFlash)
			{
				MulticastMuzzleFlash();
			}
			if (FireSound)
			{
				MulticastFireSound();
			}
		}
	}
}

void AProjectileWeapon::MulticastMuzzleFlash_Implementation()
{
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash")))
	{
		if (UWorld* World = GetWorld())
		{
			FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlash,
				SocketTransform
			);
		}
	}
}

void AProjectileWeapon::MulticastFireSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		FireSound,
		GetActorLocation()
	);
}
