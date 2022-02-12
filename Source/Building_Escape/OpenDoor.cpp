// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/PrimitiveComponent.h"
#include "OpenDoor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UOpenDoor::UOpenDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UOpenDoor::BeginPlay()
{
	Super::BeginPlay();
	InitialYaw = GetOwner()->GetActorRotation().Yaw;
	OpenAngle += InitialYaw;
	CurrentYaw = InitialYaw;
	
	if (!PressurePlate)
	{
		UE_LOG(LogTemp, Error, TEXT("Actor %s has the open door component on it, but no pressure plate set!"), *GetOwner()->GetName())
	}
}

// Called every frame
void UOpenDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TotalMassOfActors() >= MassToOpenDoor)
	{
		DoorLastOpened = GetWorld()->GetTimeSeconds();
		OpenDoor(DeltaTime);
	}
	else
	{
		if ((GetWorld()->GetTimeSeconds() - DoorLastOpened) >= DoorCloseDelay)
		{
			CloseDoor(DeltaTime);
		}
	}
}

void UOpenDoor::OpenDoor(float DeltaTime)
{
	CurrentYaw = FMath::Lerp(CurrentYaw, OpenAngle, DeltaTime * DoorOpenSpeed);
	FRotator NewRotation (0.f, CurrentYaw, 0.f);
	GetOwner()->SetActorRotation(NewRotation);
}

void UOpenDoor::CloseDoor(float DeltaTime)
{
	CurrentYaw = FMath::Lerp(CurrentYaw, InitialYaw, DeltaTime * DoorCloseSpeed);
	FRotator NewRotation (0.f, CurrentYaw, 0.f);
	GetOwner()->SetActorRotation(NewRotation);
}

float UOpenDoor::TotalMassOfActors()
{
	float TotalMass = 0.f;

	// Find all overlapping actors
	TArray<AActor*> OverlappingActors;
	if (!PressurePlate) {return TotalMass;}
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	// Add up the masses of the overlapping actors
	for (AActor* Actor : OverlappingActors)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Component Mass: %f"), Actor->FindComponentByClass<UPrimitiveComponent>()->GetMass());//Component->GetMass());
		TotalMass += Actor->FindComponentByClass<UPrimitiveComponent>()->GetMass();
		// UE_LOG(LogTemp, Warning, TEXT("Total Mass: %f"), TotalMass);
	}
	return TotalMass;
}