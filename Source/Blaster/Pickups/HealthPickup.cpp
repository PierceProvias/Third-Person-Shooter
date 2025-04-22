// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "../Characters/BlasterCharacter.h"
#include "../BlasterCharacterComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Components/SphereComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
	
	this->GetOverlapSphere()->SetSphereRadius(85.f);
	bShouldTransform = false;
}

void AHealthPickup::Destroyed()
{
	if(GetPickupEffect())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			GetPickupEffect(),
			GetActorLocation(),
			GetActorRotation()
			);
	}
	Super::Destroyed();
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if(UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent())
		{
			BuffComponent->Heal(HealAmount, HealingTime);
		}
	}
	Destroy(); 
}
