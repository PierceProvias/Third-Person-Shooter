// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../Weapons/Weapon.h"
#include "../BlasterTypes/CombatState.h"
#include "Blaster/BlasterCharacterComponents/CombatComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();

	// Offset Yaw for Strafing
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);	// Interps using shortest path (Good for FRotators)
	YawOffset = DeltaRotation.Yaw;

	// Lean
	BlasterCharacterRotationLastFrame = BlasterCharacterRotation;
	BlasterCharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(BlasterCharacterRotation, BlasterCharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	// IK for left hand for gripping weapon
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		// TODO: Align weapon to crosshairs
		// hand_r is a reference (transform is relative to hand_r)
		FName EffectorTarget = BlasterCharacter->GetEffectorTargetName();
		BlasterCharacter->GetMesh()->TransformToBoneSpace(
			EffectorTarget,
			LeftHandTransform.GetLocation(),
			FRotator::ZeroRotator,
			OutPosition,
			OutRotation
		);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(EffectorTarget, ERelativeTransformSpace::RTS_World);
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(
				RightHandTransform.GetLocation(), 
				RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
		}
		Weapon_Crosshairs_DebugLines(ActivateDebugLines);
		//SynchronizeMuzzleWithCrosshairs(LeftHandTransform.GetLocation(), OutPosition);
	} 

	bUseFABRIK			= BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	if (BlasterCharacter->IsLocallyReloading() && BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade)
	{
		bUseFABRIK = !BlasterCharacter->IsLocallyReloading();
	}
	bUseAimOffsets		= BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Unoccupied;
}

void UBlasterAnimInstance::Weapon_Crosshairs_DebugLines(bool TurnOn)
{
	if (TurnOn)
	{
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);
	}
}

/*void UBlasterAnimInstance::SynchronizeMuzzleWithCrosshairs(FVector& TraceStart, const FVector& TraceEnd)
{
	FVector MuzzelForwardVector = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World).GetLocation().ForwardVector;
	FVector CrosshairsForwardVector = BlasterCharacter->GetHitTarget().ForwardVector;

	
	//FVector MuzzelWorldLocation = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World));
	FRotator MuzzleWorldRotation = FRotator();
	EquippedWeapon->GetWeaponMesh()->GetSocketWorldLocationAndRotation(FName("MuzzleFlash"), TraceStart, MuzzleWorldRotation);

	//TraceStart = MuzzleWorldLocation;
	FVector EndPoint = MuzzelForwardVector * 33423432 + MuzzelForwardVector;

	if (UWorld* World = GetWorld())
	{
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);
	}
	
}*/
