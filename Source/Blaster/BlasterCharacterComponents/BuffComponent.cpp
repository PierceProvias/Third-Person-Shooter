// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "../Characters/BlasterCharacter.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBuffComponent::Heal(float HealAmount, float HealTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || BlasterCharacter == nullptr || BlasterCharacter->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	BlasterCharacter->SetHealth(FMath::Clamp((BlasterCharacter->GetHealth() + HealThisFrame), 0, BlasterCharacter->GetMaxHealth()));
	BlasterCharacter->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if(AmountToHeal <= 0.f || BlasterCharacter->GetHealth() >= BlasterCharacter->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
}

