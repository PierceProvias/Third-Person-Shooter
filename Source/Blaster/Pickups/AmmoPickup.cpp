// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "../Characters/BlasterCharacter.h"
#include "../BlasterCharacterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(ABlasterCharacter* Blaster = Cast<ABlasterCharacter>(OtherActor))
	{
		if(UCombatComponent* CombatComponent = Cast<UCombatComponent>(OtherComp))
		{
			CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
