// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

#include "Blaster/Weapons/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));	
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	
}

void APickup::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(PickupMesh)
	{
		RunningTime += DeltaTime;
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
		AddActorWorldRotation(FRotator(0.f, 1.f, 0.f));
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation());
	}
}

float APickup::TransformedSin()
{
	return FMath::Sin(TimeConstant * RunningTime) * Amplitude;
}

