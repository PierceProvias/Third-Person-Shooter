// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "../HUD/BlasterHUD.h"
#include "../Weapons/WeaponTypes.h"
#include "../BlasterTypes/CombatState.h"

#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterPlayerController;
class ABlasterHUD;
class UTexture2D;
class AProjectile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwitchWeapons();
	
	void DropCurrentWeapon();
	void Reload();
	
	UFUNCTION(BlueprintCallable)
	void FinishedReloading();

	UFUNCTION(BlueprintCallable)
	void FinishedThrowingGrenade();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	
protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	
	// RPCs can have params. RepNotifies cannot
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION()
	void OnRep_DropCurrentWeapon();

	UFUNCTION()
	void FireButtonPressed(bool bPressed);

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	// Called from the server
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	// Happens on all machines (server and client)
	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere, Category = "Projectile Class")
	TSubclassOf<AProjectile> GrenadeClass;

	void SwapEquippedWeapon();
	//void ActorToCharacterSocket(AActor* ActorToAttach);
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	
private:

	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	TObjectPtr<ABlasterPlayerController> BlasterController;
	TObjectPtr<ABlasterHUD> BlasterHUD;

	// Replicated so that all machines are in the correct animation pose
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	TObjectPtr<AWeapon> SecondaryWeapon;
	
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming;
	
	UFUNCTION()
	void OnRep_Aiming();
	
	bool bAimButtonPressed;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
	 * HUD and crosshairs
	 */
	
	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	
	float CrosshairShootingFactor;

	FVector HitTarget;

	/*
	 * Aiming and FOV
	 */

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	* Automatic Fire
	*/

	FTimerHandle FireTimer;

	
	bool bCanFire = true;
	void FireTimerStart();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;	// TMap isn't replicated!

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 MaxCarriedAmmo = 500;
	/*
	* Starting ammo for each weapon type
	*/
	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingAKAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingPistolAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere, Category = "Starting Ammo")
	int32 StartingGrenadeLauncherAmmo = 0;

	void InitCarriedAmmo();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateWeapon2DTextures();

	UPROPERTY(EditAnywhere)
	float GrenadeSpawnDifferential = 1.f;


	UPROPERTY(ReplicatedUsing = OnRep_GrenadeCount)
	int32 GrenadeCount = 4;

	UFUNCTION()
	void OnRep_GrenadeCount();
	
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenadeCount();

public:
	FORCEINLINE int32 GetGrenadeCount() { return GrenadeCount; }
	bool ShouldSwitchWeapons();
};
