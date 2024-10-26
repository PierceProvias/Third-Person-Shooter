// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "EnhancedInputSubsystems.h"

#include "../PlayerController/BlasterPlayerController.h"
#include "../Weapons/Weapon.h"
#include "../Characters/BlasterCharacter.h"
#include "../Weapons/Projectile.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		
		if (BlasterCharacter->GetCamera())
		{
			DefaultFOV = BlasterCharacter->GetCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (BlasterCharacter->HasAuthority())
		{
			InitCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, GrenadeCount);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (BlasterCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		BlasterCharacter->ShowSniperScopeWidget(bIsAiming);
	}
}

// _Implementation is for RPC
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (BlasterCharacter && EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;
		UpdateWeapon2DTextures();
		PlayEquipWeaponSound();
	
		/*
		
			TArray<FInputAxisKeyMapping> AxisMappings;
			FName AxisName;
			UInputSettings::GetInputSettings()->GetAxisMappingByName(AxisName, AxisMappings);
			for (auto Axis : AxisMappings)
			{
				if (BlasterController->WasInputKeyJustPressed(Axis.Key))
				{
					return true;
				}
			}
			return false;
			if (BlasterCharacter->GetInputAxisValue(FName("DropWeaponAction")))
			{
				//OnRep_DropCurrentWeapon();
			}
		*/
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (BlasterCharacter == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	SwapEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);

	// Set equipped weapon to controlled pawn 
	EquippedWeapon->SetOwner(BlasterCharacter);
	EquippedWeapon->SetHUDAmmo();
	UpdateWeapon2DTextures();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound();
	ReloadEmptyWeapon();

	// Disable orient to movement so we can strafe
	// NOTE: This will only be done on the server therefore we need to use a RepNotify
	BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BlasterCharacter->bUseControllerRotationYaw = true;

}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (BlasterCharacter && BlasterCharacter->GetAttachedGrenade())
	{
		BlasterCharacter->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::PlayEquipWeaponSound()
{
	if (BlasterCharacter && EquippedWeapon && EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			BlasterCharacter->GetActorLocation()
		);
	}
}

void UCombatComponent::DropCurrentWeapon()
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) return;
	
	EquippedWeapon->GetWeaponMesh()->AddImpulse(
		BlasterCharacter->GetActorForwardVector() * WEAPON_FORWARD_DROP_VELOCITY +
		BlasterCharacter->GetActorUpVector() * WEAPON_UPWARD_DROP_VELOCITY
	);
	EquippedWeapon->Dropped();
	//EquippedWeapon->Swapped();
	//BlasterCharacter->DestroyConstructedComponents();
	
}

void UCombatComponent::OnRep_DropCurrentWeapon()
{
	if (BlasterCharacter && EquippedWeapon)
	{
		EquippedWeapon->GetWeaponMesh()->AddImpulse(
			BlasterCharacter->GetActorForwardVector() * WEAPON_FORWARD_DROP_VELOCITY + 
			BlasterCharacter->GetActorUpVector() * WEAPON_UPWARD_DROP_VELOCITY
		);
		EquippedWeapon->Dropped();
		//EquippedWeapon->Swapped();
		//BlasterCharacter->DestroyConstructedComponents();
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;	// Locally controlled
	
	if (EquippedWeapon)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			bCanFire = false;
			CrosshairShootingFactor = 0.75f;
		}
		FireTimerStart();

	}
	return;
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// Trace from the center of the screen
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // Center for screenspace
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;	
	
	// Gte crosshair location in world Space
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		// For line trace
		FVector Start = CrosshairWorldPosition;

		if (BlasterCharacter)
		{
			float DistanceToCharacter = (BlasterCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
			
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->Controller == nullptr) return;
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterController->GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter		= BlasterCharacter->GetEquippedWeapon()->GetCrosshairs_Center();
				HUDPackage.CrosshairsLeft		= BlasterCharacter->GetEquippedWeapon()->GetCrosshairs_Left();
				HUDPackage.CrosshairsRight		= BlasterCharacter->GetEquippedWeapon()->GetCrosshairs_Right();
				HUDPackage.CrosshairsTop		= BlasterCharacter->GetEquippedWeapon()->GetCrosshairs_Top();
				HUDPackage.CrosshairsBottom		= BlasterCharacter->GetEquippedWeapon()->GetCrosshairs_Bottom();
			}
			else
			{
				HUDPackage.CrosshairsCenter		= nullptr;
				HUDPackage.CrosshairsLeft		= nullptr;
				HUDPackage.CrosshairsRight		= nullptr;
				HUDPackage.CrosshairsTop		= nullptr;
				HUDPackage.CrosshairsBottom		= nullptr;
			}

			// Calculate crosshair spread
			// Map [0, 600] -> [0, 1] for movement speed

			FVector2D WalksSpeedRange(0.f, BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = BlasterCharacter->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalksSpeedRange, VelocityMultiplierRange, Velocity.Size());
			
			if (BlasterCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 
				0.5f +
				CrosshairVelocityFactor + 
				CrosshairInAirFactor + 
				CrosshairAimFactor +
				CrosshairShootingFactor;

			BlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::HandleReload()
{
	BlasterCharacter->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::ServerReload_Implementation()
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
	UpdateAmmoValues();
}

void UCombatComponent::UpdateAmmoValues()
{

	if (EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::UpdateWeapon2DTextures()
{
	// Where is this used
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		UTexture2D* EquippedWeaponTexture2D = EquippedWeapon->GetWeaponTexture2D();
		if (EquippedWeapon && EquippedWeaponTexture2D)
		{
			BlasterController->SetHUDCarriedWeaponTexture(EquippedWeaponTexture2D);
			UE_LOG(LogTemp, Warning, TEXT("Texture: %s"), *EquippedWeaponTexture2D->GetName());
		}
	} 
}

void UCombatComponent::OnRep_GrenadeCount()
{
	UpdateHUDGrenadeCount();
}

void UCombatComponent::UpdateHUDGrenadeCount()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterController->SetHUDPrimaryGrenade(GrenadeCount);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (BlasterCharacter && !BlasterCharacter->IsLocallyControlled())
		{
			BlasterCharacter->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);	// Not locally controlled
		}
		break;
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (BlasterCharacter && BlasterCharacter->GetCamera())
	{
		BlasterCharacter->GetCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::FireTimerStart()
{
	if (EquippedWeapon == nullptr || BlasterCharacter == nullptr) return;
	BlasterCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay 
	);
	ReloadEmptyWeapon();
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{

		if (APlayerController* PlayerController = Cast<APlayerController>(BlasterCharacter->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				//Subsystem->AddPlayerMappedKey(FName("CharacterMappingContext"), FKey(EKeys::LeftMouseButton), &FModifyContextOptions::bForceImmediately);
				//Subsystem->ClearAllMappings();
				//Subsystem->AddMappingContext(LookOnlyMappingContext, 0);
				FKey FireKey = Subsystem->GetPlayerMappedKey(FName("FireAction"));

				//EInputActionValueType Action EInputActionValueType::Boolean, 0.f);
				EInputActionValueType ValueType = EInputActionValueType::Boolean;
				FInputActionValue ActionValue;
				ActionValue.ConvertToType(ValueType);

				TArray<UInputModifier*> Modifiers;
				TArray<UInputTrigger*> Triggers;
				//UInputTriggerPressed* Pressed = EInputEvent::IE_Pressed;
				
				//if (Pressed)
				//{
					//Triggers.Emplace(Pressed);

				//}
			
					// TODO: Change FireAction Input Mapping Key trigger to pressed when !automatic weapon
				Subsystem->InjectInputForAction(BlasterCharacter->GetFireAction(), ActionValue, Modifiers, Triggers);
				Fire();
			}
		}
	}
}

bool UCombatComponent::CanFire()
{
	if (!EquippedWeapon) return false;
	if (EquippedWeapon->IsEmpty()) return false;
	if (!bCanFire) return false;
	if (CombatState != ECombatState::ECS_Unoccupied) return false;
	return true;
	
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AK47, StartingAKAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);   // Runs on server and all clients
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (BlasterCharacter && CombatState == ECombatState::ECS_Unoccupied)
	{
		BlasterCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		ServerReload();
	}
}

void UCombatComponent::ThrowGrenade()
{
	if(GrenadeCount == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (BlasterCharacter)
	{
		BlasterCharacter->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	if (BlasterCharacter && !BlasterCharacter->HasAuthority())
	{
		ServerThrowGrenade();	
	}
	if (BlasterCharacter && BlasterController->HasAuthority())
	{
		GrenadeCount = FMath::Clamp(GrenadeCount - 1, 0, MaxGrenades);
		UpdateHUDGrenadeCount();
	}
}

void UCombatComponent::SwapEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Swapped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, BlasterCharacter->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("LeftHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, BlasterCharacter->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(GrenadeCount == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (BlasterCharacter)
	{
		BlasterCharacter->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	GrenadeCount = FMath::Clamp(GrenadeCount - 1, 0, MaxGrenades);
	UpdateHUDGrenadeCount();
}

void UCombatComponent::FinishedReloading()
{
	if (BlasterCharacter == nullptr) return;
	if (BlasterCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
}

void UCombatComponent::FinishedThrowingGrenade()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (BlasterCharacter && GrenadeClass && BlasterCharacter->GetAttachedGrenade())
	{
		const FVector StartingLocation = BlasterCharacter->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FVector ToTargetNormalized = (Target - StartingLocation).GetSafeNormal();
		FVector AdjustedStartingLocation = StartingLocation + (ToTargetNormalized * GrenadeSpawnDifferential);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = BlasterCharacter;
		SpawnParams.Instigator = BlasterCharacter;
		if (UWorld* World = GetWorld())
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				AdjustedStartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}
