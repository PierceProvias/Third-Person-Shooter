#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//
	// Replication notifies
	//

	virtual void OnRep_Score() override;

	// RepNotifies must be a UFUNCTION unless overrided
	UFUNCTION()
	virtual void OnRep_Deaths();
	
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);
	
private:

	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	TObjectPtr<ABlasterPlayerController> BlasterController;
	
	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
};
