// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

class ACasing;
class UTexture2D;
class ABlasterCharacter;
class ABlasterPlayerController;
class USoundCue;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UPickupWidget;

#define LIST_DATA_ACCESSOR(DataType, PropertyName) \
	FORCEINLINE DataType Get##PropertyName() const { return PropertyName; } \
	void Set##PropertyName(DataType In##PropertyName) {PropertyName = In##PropertyName; } 

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial					UMETA(DisplayName = "Initial State"),
	EWS_Equipped				UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary		UMETA(DisplayName = "Equipped Secondary"),
	EWS_Swapped					UMETA(DisplayName = "Swapped"),
	EWS_Dropped					UMETA(DisplayName = "Dropped"),

	EWS_MAX						UMETA(DisplayName = "DefaultMAX")  
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan					UMETA(DisplayName = "Hit Scan"),
	EFT_Projectile				UMETA(DisplayName = "Projectile"),
	EFT_Shotgun					UMETA(DisplayName = "Shotgun"),

	EFT_MAX						UMETA(DisplayName = "DefaultMAX") 
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void ShowPickupWidget(bool bShowWidget);
	void SetPickupWidgetWeaponInfo(bool bSetInfo);
	void PlayPickupWidgetAnimation();
	// void SetKillConfirmedSprite(AWeapon* Weapon);
	virtual void Fire(const FVector& HitTarget);
	void Swapped();
	void Dropped();
	
	/*
	* Automatic fire
	*/
	
	// Determines the rate of fire
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;

	void SetHUDAmmo();
	
	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> EquipSound;

	
	/*
	* Enable or disable custom depth
	*/

	void EnableCustomDepth(bool bEnable);
	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere, Category = "Fire Type")
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
	
	FVector TraceEndWithScatter(const FVector& HitTarget, bool DebugEnabled = false);
	
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnEquippedSecondary();
	virtual void OnSwapped();

	// We are only generating overlap events on the server
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	); 

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	TObjectPtr<UMaterialInstanceDynamic> WidgetMaterialInstance;


	/*
	* Trace end with scatter
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;


	
private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	TObjectPtr<UTexture2D> WeaponTexture2D;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	TObjectPtr<UTexture2D> KillConfirmedSprite;

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	TArray<UTexture2D*> KillConfirmedSprites;	
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<USphereComponent> AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<ACasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MagCapacity;

	// Number of unprocessed server requests for Ammo
	// Incremented in SpendRound(), decremented in ClientUpdateAmmo()
	int32 Sequence = 0;

	TWeakObjectPtr<ABlasterCharacter> BlasterOwnerCharacter;
	TWeakObjectPtr<ABlasterPlayerController> BlasterOwnerController;

	void SpendRound();

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	/*
	* Textures for the weapon crosshairs
	*/
	
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsBottom;

	/*
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;
	
	TWeakObjectPtr<UPickupWidget> PickupWidgetInstance;

public:	
	
	void SetWeaponState(EWeaponState State);
	bool IsEmpty();
	bool IsFull();
	
	LIST_DATA_ACCESSOR(USphereComponent*, AreaSphere);
	LIST_DATA_ACCESSOR(USkeletalMeshComponent*, WeaponMesh);
	LIST_DATA_ACCESSOR(UTexture2D*, KillConfirmedSprite);
	LIST_DATA_ACCESSOR(UTexture2D*, WeaponTexture2D)
	LIST_DATA_ACCESSOR(TArray<UTexture2D*>, KillConfirmedSprites)
	LIST_DATA_ACCESSOR(float, ZoomedFOV)
	LIST_DATA_ACCESSOR(float, ZoomInterpSpeed)
	LIST_DATA_ACCESSOR(EWeaponType, WeaponType)
	LIST_DATA_ACCESSOR(int32, Ammo)
	LIST_DATA_ACCESSOR(int32, MagCapacity)
	
	FORCEINLINE UTexture2D* GetCrosshairs_Center() const { return CrosshairsCenter; }
	FORCEINLINE UTexture2D* GetCrosshairs_Left() const { return CrosshairsLeft; }
	FORCEINLINE UTexture2D* GetCrosshairs_Right() const { return CrosshairsRight; }
	FORCEINLINE UTexture2D* GetCrosshairs_Top() const { return CrosshairsTop; }
	FORCEINLINE UTexture2D* GetCrosshairs_Bottom() const { return CrosshairsBottom; }
};
