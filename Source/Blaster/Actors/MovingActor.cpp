// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingActor.h"
#include "Components/BoxComponent.h"
#include "Components/InterpToMovementComponent.h"

AMovingActor::AMovingActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	RootComponent = BoxCollider;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(RootComponent);

	InterpMovementComponent = CreateDefaultSubobject<UInterpToMovementComponent>(TEXT("InterpMovementComponent"));
	InterpMovementComponent->Duration = 5.f;
	InterpMovementComponent->bSweep = true;
	InterpMovementComponent->BehaviourType = EInterpToBehaviourType::PingPong;
}

void AMovingActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Setting up the control points for movement down our path
	InterpMovementComponent->ControlPoints.Add(FInterpControlPoint(FVector(0.f, 0.f, 0.f), true));
	for (int i = 0; i < ThePath.Num(); i++)
	{
		InterpMovementComponent->ControlPoints.Add(FInterpControlPoint(FVector(ThePath[i]), true));
	}
	InterpMovementComponent->FinaliseControlPoints();
}

void AMovingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

