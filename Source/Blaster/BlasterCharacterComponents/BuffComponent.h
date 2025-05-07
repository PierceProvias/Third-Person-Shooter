// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;
	void Heal(float HealAmount, float HealTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	bool bHealing		= false;
	bool bReplenishing	= false;
	
	float HealingRate				= 0.f;
	float AmountToHeal				= 0.f;
	float ReplenishingRate			= 0.f;
	float ShieldReplenishAmount		= 0.f;
	
public:	
	

		
};
