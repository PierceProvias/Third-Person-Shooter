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
	
	UFUNCTION(Client, Reliable)
	void Client_SwitchToAttackerCamera(ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);

	UFUNCTION(Server, Reliable)
	void Server_SwitchToAttackerCamera(ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

};
