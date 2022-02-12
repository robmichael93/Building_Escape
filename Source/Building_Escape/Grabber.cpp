// Fill out your copyright notice in the Description page of Project Settings.

#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Grabber.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandle();
	SetupInputComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) {return;}
	// If the physics handle has something grabbed (an object is in the player's grasp)
	if (PhysicsHandle->GrabbedComponent)
	{
		// set the location for that object we're moving around
		PhysicsHandle->SetTargetLocation(GetPlayersReachPosition());
	}
}

// Check to make sure this object has a physics handle component & log an error if it does not
void UGrabber::FindPhysicsHandle()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	
	if (!PhysicsHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("%s does not have a valid physics handle!"), *GetOwner()->GetName());
	} 
}

// Bind the Grab & Release functions to the Grab keybind for when it is pressed & released
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();

	if (!InputComponent) {return;}
	if (InputComponent)
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	} 
}

FVector UGrabber::GetPlayersWorldPosition() const
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);

	return PlayerViewPointLocation;
}

FVector UGrabber::GetPlayersReachPosition() const
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);

	return (PlayerViewPointLocation + (PlayerViewPointRotation.Vector() * Reach));

}

// Find the grabble object at the location of the player's reach (or return nothing if not grabble or not touching anything)
FHitResult UGrabber::GetFirstPhysicsBodyInReach() const
{
	FHitResult Hit;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());

	// Will only generate "hits" on objects that have a physics body component
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit, 
		GetPlayersWorldPosition(), 
		GetPlayersReachPosition(), 
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody), 
		TraceParameters
	);

	return Hit;
}

// Attempt to grab an object when the Grab keybind is pressed
// Check to see if anything that can be grabbed is present at the player's reach location when pressed
// If so, attach that object to the physics handle (the player's grasp) so it can be moved around
void UGrabber::Grab()
{
	FHitResult HitResult = GetFirstPhysicsBodyInReach();
	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();
	AActor* ActorHit = HitResult.GetActor();

	if (!ActorHit) {return;}
	// If we hit something with the player's reach then attach the physics handle to it (i.e. grab it).
	if (HitResult.GetActor())
	{
		PhysicsHandle->GrabComponentAtLocation(ComponentToGrab, NAME_None, GetPlayersReachPosition());
	}
}

// Simply release the object from the physics handle (the player's grasp)
void UGrabber::Release()
{
	if (!PhysicsHandle) {return;}
	PhysicsHandle->ReleaseComponent();
}