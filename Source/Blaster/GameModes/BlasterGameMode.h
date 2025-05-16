#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	// Match duration has been reached. Display winner and begin cooldown timer.
}


UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void SwitchToAttackerCamera(ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:

	UPROPERTY(EditDefaultsOnly, Category = "GameTimers")
	float CountdownTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameTimers")
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameTimers" )
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameTimers")
	float CooldownTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "GameTimers")
	float LevelStartingTime = 0.f;
	
public:

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	FORCEINLINE float GetWarmupTime() const { return WarmupTime; }
	FORCEINLINE float GetMatchTime() const { return MatchTime; }
	FORCEINLINE float GetCooldownTime() const { return CooldownTime; }
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }
};
