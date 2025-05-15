#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Camera/CameraShakeBase.h"

#include "../Weapons/Weapon.h"
#include "../BlasterCharacterComponents/CombatComponent.h"
#include "../BlasterCharacterComponents/BuffComponent.h"
#include "../Characters/BlasterAnimInstance.h"
#include "../Blaster.h"
#include "../PlayerController/BlasterPlayerController.h"
#include "../GameModes/BlasterGameMode.h"
#include "../PlayerState/BlasterPlayerState.h"
#include "../Weapons/WeaponTypes.h"
#include "../Cameras/DamageCamera.h"
#include "../HUD/KillConfirmed.h"
#include "PauseMenu.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	

	// Controller Rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	// Character Movement Component
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Overhead Widget
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead Widget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Combat component
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	CombatComponent->SetIsReplicated(true); // No need to register in Props array (components never do)

	// Buff component
	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff Component"));
	BuffComponent->SetIsReplicated(true); // No need to register in Props array (components never do)
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// Grenade component
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	KillConfirmedWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Kill Confirmed Widget"));
	KillConfirmedWidget->SetupAttachment(RootComponent);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
			bInputsSet = true;
		}
	}
	UpdateHUDHealth();
	UpdateHUDShield();
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	if (KillConfirmedWidget)
	{
		UUserWidget* WidgetInstance = KillConfirmedWidget->GetUserWidgetObject();
		KillConfirmedWidgetInstance = KillConfirmedWidgetInstance == nullptr ? Cast<UKillConfirmed>(WidgetInstance) : KillConfirmedWidgetInstance;
		ShowKillConfirmedWidget(false);
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		if (CombatComponent->EquippedWeapon->bDestroyWeapon)
		{
			CombatComponent->EquippedWeapon->Destroy();	
		}
		else
		{
			CombatComponent->EquippedWeapon->Swapped();		// TODO: Need to be dropped here (create function for dropping current weapon)
		}
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this, 
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();
	ShowKillConfirmedWidget(true);

	GetWorldTimerManager().SetTimer(
		KillConfirmedTimer,
		this, 
		&ABlasterCharacter::KillConfirmedTimerFinished,
		KillConfirmedAnnoucmentTime
	);
	
	// Start dissolve effect when eliminated
	if (DissolveMaterialInstance && ToonShaderPostProcessInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		
		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance);
		
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

		// TODO: Turn off post-process effect for elimmed player while playing dissolve track
		// DynamicToonShaderPostProcessInstance = UMaterialInstanceDynamic::Create(ToonShaderPostProcessInstance, this);
		// DynamicToonShaderPostProcessInstance->SetScalarParameterValue(TEXT("MinThickness DO"), 0.f);
		// DynamicToonShaderPostProcessInstance->SetScalarParameterValue(TEXT("MaxThickness DO"), 0.f);
		// GetMesh()->SetMaterial(2, DynamicToonShaderPostProcessInstance);

	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();			// Prevent movement
	GetCharacterMovement()->StopMovementImmediately();	// Prevent rotation 

	if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn ELim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + ElimBotHoverDistance);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperScope = IsLocallyControlled() &&
		CombatComponent &&
		CombatComponent->bAiming &&
		CombatComponent->EquippedWeapon &&
		CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper;
	
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (CombatComponent && CombatComponent->EquippedWeapon && BlasterGameMode && BlasterGameMode->GetCountdownTime() <= 0.f)
	{
		CombatComponent->EquippedWeapon->Destroy();	
	}
}



void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);
	if (HasAuthority() && Controller && !bInputsSet)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(CharacterMappingContext, 0);
				bInputsSet = true;
			}
		}
	}
	// When character is backed up against a wall 
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;  // If we movement hasn't been updated, it will update now
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(DropWeaponAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::DropWeaponPressed);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::PauseMenuPressed);
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::ThrowGrenadeButtonPressed);
		// TODO: Find Jump animation for unequipped pose (Current one lags)
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	// Input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		// Find out which way is forward

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		// Get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	// Input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABlasterCharacter::Jump()
{
	Super::Jump();
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (CombatComponent)
	{
		if (HasAuthority() && OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
			// SetKillConfirmedSprite(OverlappingWeapon); Fix Bug!
		}
		else
		{
			// If we don't have authority, the RPC will let the server know to equip the weapon for the client 
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (!bIsCrouched)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(Value.Get<bool>());
	}
}

void ABlasterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(Value.Get<bool>());
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void ABlasterCharacter::DropWeaponPressed()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		if (HasAuthority())
		{
			CombatComponent->DropCurrentWeapon();
		}
		else
		{
			ServerDropWeaponButtonPressed();
		}
	}

	// TODO: Detach weapon mesh from right hand socket 
	// TODO: Probably best to move this to the combat component
	
}

void ABlasterCharacter::PauseMenuPressed()
{
	// TODO: Leave 
	/*
		if (IsLocallyControlled() && PauseMenu)
	UPauseMenu* PauseMenu = CreateWidget<UPauseMenu>(this, UPauseMenu::StaticClass(), FName("WBP_PauseMenu"));
	if (IsLocallyControlled() && PauseMenu)
	{
		PauseMenu->MenuSetup();
		UE_LOG(LogTemp, Warning, TEXT("Paused"));
	}
		{
			PauseMenu->MenuSetup();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Paused NOT WORKING"));
		}
	*/
}

void ABlasterCharacter::ThrowGrenadeButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// Aim Offsets
	// Standing still 
	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	// Running or jumping
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from [270, 360) to [-90, 0)
		// Used to correct pitch on client viewed by the server
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr && CombatComponent->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
		OverlappingWeapon->PlayPickupWidgetAnimation();
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon.Get());
		}
		else if (CombatComponent->ShouldSwitchWeapons())
		{
			CombatComponent->SwitchWeapons();
		}
	}
}

void ABlasterCharacter::ServerDropWeaponButtonPressed_Implementation()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->DropCurrentWeapon();
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((Camera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::KillConfirmedTimerFinished()
{
	ShowKillConfirmedWidget(false);
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeLine)
	{
		DissolveTimeLine->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeLine->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
			OverlappingWeapon->PlayPickupWidgetAnimation();
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

void ABlasterCharacter::ShowKillConfirmedWidget(bool bShowWidget)
{
	if (KillConfirmedWidget && KillConfirmedWidgetInstance)
	{
		KillConfirmedWidget->SetVisibility(bShowWidget);
		UWidgetAnimation* KillConfirmedAnimation = KillConfirmedWidgetInstance->KillConfirmedAnimation.Get();
		if (KillConfirmedWidgetInstance->KillConfirmedAnimation.IsValid() && KillConfirmedWidgetInstance->KillConfirmedImage.IsValid())
		{
			KillConfirmedWidgetInstance->PlayAnimation(KillConfirmedAnimation);
			KillConfirmedWidgetInstance->KillConfirmedImage->SetBrushFromTexture(KillConfirmedSprite);
		}
	}
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return CombatComponent->CombatState;
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{	
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (CombatComponent == nullptr) return FVector();
	return CombatComponent->HitTarget;
}



void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->BlasterCharacter = this;
	}
	if(BuffComponent)
	{
		BuffComponent->BlasterCharacter = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_AK47:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("AssaultRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("AssaultRifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || ElimMontage) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage && !ElimMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		//FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection("FromFront");
		UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage"));
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if(bElimmed) return;

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= DamageToHealth)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}
	
	// Since this is a replicated variable and sent to all clients, we do not need a MultiHit RPC. Variable replication is less expensive

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health != 0.f)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			if (TSubclassOf<UDamageCamera> DamageCameraClass = UDamageCamera::StaticClass())
			{
				BlasterPlayerController->ClientStartCameraShake(DamageCameraClass);
			}
		}
	}
	
	if (Health == 0.f)
	{
		if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			if (BlasterPlayerController && AttackerController)
			{
				BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);	
				// TODO: Black/White Screen for elimmed character 
			}
		}
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::UpdateHUDWeaponTexture2D(AWeapon* Weapon)
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Weapon)
	{
		BlasterPlayerController->SetHUDCarriedWeaponTexture(Weapon->GetWeaponTexture2D());
	}
}

void ABlasterCharacter::SetKillConfirmedSprite(AWeapon* Weapon)
{
	if (Weapon->GetKillConfirmedSprites().Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("KillConfirmedSprites"));
		for (const auto& texture : Weapon->GetKillConfirmedSprites())
		{
			UE_LOG(LogTemp, Warning, TEXT("%p"), texture);
		}
		uint32 Selection = FMath::RandRange(0, Weapon->GetKillConfirmedSprites().Num() - 1);
		if (Weapon->GetWeaponType() <= EWeaponType::EWT_GrenadeLauncher)
		{
			KillConfirmedSprite = Weapon->GetKillConfirmedSprites()[Selection];
		}
		else
		{
			KillConfirmedSprite = Weapon->GetKillConfirmedSprites()[Selection];	
		}
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDeaths(0);
		}
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	// GetGameMode returns NULL if not on server
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (CombatComponent)
		{
			CombatComponent->EquipWeapon(StartingWeapon);
			UpdateHUDWeaponTexture2D(StartingWeapon);
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Registering variable below to be replicated only to owning pawn. Only the client will see this when overlapping with the weapon
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health); // Health will be replicated down to the character
	DOREPLIFETIME(ABlasterCharacter, Shield); // Shield will be replicated down to the character
}
