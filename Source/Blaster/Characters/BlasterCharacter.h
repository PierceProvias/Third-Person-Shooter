#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"

#include "../Interfaces/InteractWithCrosshairsInterface.h"
#include "../BlasterTypes/TurningInPlace.h"
#include "../BlasterTypes/CombatState.h"

#include "BlasterCharacter.generated.h"

class UPostProcessComponent;
class UWidgetAnimation;
class UKillConfirmed;
class UDamageCamera;
class UBuffComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInputComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;
class AController;
class USoundCue;
class ABlasterPlayerState;
class UPauseMenu;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Must be overrided in any class that we choose to replicate variables
	// This is where we register variables to be replicated
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Used to construct components
	virtual void PostInitializeComponents() override;

	//TSubclassOf<UDamageCamera> DamageCamera;
	
	/*
	* Play animation montages
	*/
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	void PlayHitReactMontage();
	
	// Since SimProxies Tick is off, Used for replication of our root component's position and velocity (Inherited from Actor.h)
	virtual void OnRep_ReplicatedMovement() override;
	
	void Elim();

	// Handles player when eliminated
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	virtual void Destroyed() override;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void UpdateHUDWeaponTexture2D(AWeapon* Weapon);
	void SetKillConfirmedSprite(AWeapon* Weapon);
protected:
	
	virtual void BeginPlay() override;
	void RotateInPlace(float DeltaTime);

	/*
	* Callbacks for input
	*/

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed(const FInputActionValue& Value);
	void FireButtonPressed(const FInputActionValue& Value);
	void ReloadButtonPressed();
	void DropWeaponPressed();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	
	UFUNCTION(BlueprintCallable)
	void PauseMenuPressed();

	void ThrowGrenadeButtonPressed();

	/*
	* Aim Offsets
	*/
	
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	
	// Simulates turning for proxies
	// Allows for higher performance since we don't need to know the proxy's aim offsets
	// Prevents rotation of root bone for simulated proxies
	void SimProxiesTurn();
	
	// Bound to OnTakeAnyDamage delegate (needs UFUNCTION macro)
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	

	// Poll for any relevant classes and init our HUD
	void PollInit();

	
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TObjectPtr<UWidgetComponent> KillConfirmedWidget;
	
	void SpawnDefaultWeapon();

private:

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TObjectPtr<UKillConfirmed> KillConfirmedWidgetInstance;

	UPROPERTY(EditAnywhere, Category = "CameraBoom")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> CharacterMappingContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> EquipAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DropWeaponAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ThrowGrenadeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	// Replication is when an object changes on the server, we want it to be "replicated" to all clients
	// "OnRep_OverlappingWeapon" will be called on the client when overlapping weapon replicates to that client 
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;

	// REPNOTIFY: Only called when variable's value gets replicated
	// Called automatically when variable is replicated therefore no params
	// NOTE: Repnotifies do not get called on the server. They only get called when the variable replicates
	// Replication works from server to clients
	// Must have a UFUNCTION
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBuffComponent> BuffComponent;
	
	// Remote Procedure Calls (RPC) allow for clients to send information to the server
	// A RPC will be called from the client and executed on the server (AKA a server RPC)
	// Reliable RPCs are guaranteed to be executed. Unreliable RPCs can potentially be dropped
	// Use reliable RPCs sparringly (Not in Tick function)
	// NOTE: RPCs can be called from the server and propagated to the clients
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerDropWeaponButtonPressed();
	
	// Used for calculations
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/*
	* Animation Montages
	*/

	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UAnimMontage> ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UAnimMontage> ElimMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	TObjectPtr<UAnimMontage> ThrowGrenadeMontage;

	void HideCameraIfCharacterClose();
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/*
	* Player health (Implemented in the character class instead of the player state due to faster replication)
	*/
	
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/*
	* Player shield (Implemented in the character class instead of the player state due to faster replication)
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	TObjectPtr<ABlasterPlayerController> BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;
	FTimerHandle KillConfirmedTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	float ElimDelay = 3.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	float KillConfirmedAnnouncementTime = 1.f;

	// Callback function for ElimTimer
	void ElimTimerFinished();

	void KillConfirmedTimerFinished();

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	TObjectPtr<UTexture2D> KillConfirmedSprite;
	

	/*
	* Dissolve Effect
	*/

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	TObjectPtr<UTimelineComponent> DissolveTimeLine;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere, Category = "Elim")
	TObjectPtr<UCurveFloat> DissolveCurve;

	// UTimelineComponent callback function
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elim")
	TObjectPtr<UMaterialInstanceDynamic> DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Elim")
	TObjectPtr<UMaterialInstance>  DissolveMaterialInstance;

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elim")
	TObjectPtr<UMaterialInstanceDynamic> DynamicToonShaderPostProcessInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Elim")
	TObjectPtr<UMaterialInstance>  ToonShaderPostProcessInstance;

	
	/*
	* Elim Bot (must be spawned locally)
	*/

	UPROPERTY(EditAnywhere, Category = "Elim")
	TObjectPtr<UParticleSystem> ElimBotEffect;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	TObjectPtr<UParticleSystemComponent> ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = "Elim")
	TObjectPtr<USoundCue> ElimBotSound;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
	float ElimBotHoverDistance = 200.f;

	TWeakObjectPtr<ABlasterPlayerState> BlasterPlayerState;

	bool bInputsSet = false;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> AttachedGrenade;

	UPROPERTY(EditAnywhere, Category = "FABRIK")
	FName EffectorTarget;

	// Default weapon

	UPROPERTY(EditAnywhere, Category = "Default Weapon")
	TSubclassOf<AWeapon> DefaultWeaponClass;
public:

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	void ShowKillConfirmedWidget(bool bShowWidget);

	FORCEINLINE auto GetDefaultWeaponClass() const {return DefaultWeaponClass;}
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE float GetElimDelay() const { return ElimDelay; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE UInputMappingContext* GetCharacterInputMappingContext() const { return CharacterMappingContext; }
	FORCEINLINE UInputAction* GetFireAction() const { return FireAction; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const {return CombatComponent;}	
	FORCEINLINE UBuffComponent* GetBuffComponent() const {return BuffComponent;}
	FORCEINLINE FName GetEffectorTargetName() const { return EffectorTarget; }
	FORCEINLINE FTimerHandle GetElimTimer() const { return ElimTimer; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UTexture2D* GetKillConfirmedSprite() const {return KillConfirmedSprite; }
	
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE void SetHealth(float Amount) {Health = Amount;}
	FORCEINLINE void SetShield(float Amount) {Shield = Amount;}
	
	ECombatState GetCombatState() const;
	AWeapon* GetEquippedWeapon() const;
	FVector GetHitTarget() const;
	bool IsLocallyReloading() const;

};
