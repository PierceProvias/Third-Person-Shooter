#pragma once

#define TRACE_LENGTH					80'000
#define WEAPON_FORWARD_DROP_VELOCITY	500.f
#define WEAPON_UPWARD_DROP_VELOCITY		125.f

#define CUSTOM_DEPTH_PURPLE				250
#define CUSTOM_DEPTH_BLUE				251
#define CUSTOM_DEPTH_TAN				252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_RocketLauncher		UMETA(DisplayName = "Rocket Launcher"),
	EWT_GrenadeLauncher		UMETA(DisplayName = "Grenade Launcher"),
	EWT_AssaultRifle		UMETA(DisplayName = "Assault Rifle"), 
	EWT_AK47				UMETA(DisplayName = "AK-47"),
	EWT_Pistol				UMETA(DisplayName = "Pistol"),
	EWT_SMG					UMETA(DisplayName = "SMG"),
	EWT_Shotgun				UMETA(DisplayName = "Shotgun"),
	EWT_Sniper				UMETA(DisplayName = "Sniper"),

	EWT_MAX					UMETA(DisplayName = "DefaultMAX")
};