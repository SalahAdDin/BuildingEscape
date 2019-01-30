// Fill out your copyright notice in the Description page of Project Settings.

#include "Grabber.h"
#include "BuildingEscape.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/Components/InputComponent.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"


// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandleComponent();
	SetupInputComponent();

	// ...
}

// Look for attached Physics Handle
void UGrabber::FindPhysicsHandleComponent()
{
	// Look for attached physics handler
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s missing physics handle component"), *GetOwner()->GetName())
	}
}


// Look for attached Input Component (only appears at run time)
void UGrabber::SetupInputComponent()
{
	// Look for attached input component (only appears in running time)
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();

	if (InputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Input component found"));

		//Bind the input axis
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s missing input component"), *GetOwner()->GetName())
	}
}

void UGrabber::Grab()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab pressed"))

	// LINE TRACE and see if we reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();	// gets the mesh in our case
	auto ActorHit = HitResult.GetActor();

	// If it hit something then attach a physics handle
	if (ActorHit)
	{
		// attach physics handle
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			ComponentToGrab,
			NAME_None,	// no bones needed
			ActorHit->GetActorLocation(),
			ActorHit->GetActorRotation()
		);
	}
}

void UGrabber::Release()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab released"))

	PhysicsHandle->ReleaseComponent();
}

FHitResult UGrabber::GetFirstPhysicsBodyInReach() const
{
	/// Line-trace (AKA ray-cast) out to reach distance
	FHitResult HitResult;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	FTwoVectors TracePoints = GetLineTracePoints();
	
	//with C++17
	//auto [Start, End] = GetLineTracePoints();
	GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		TracePoints.v1, //Start
		TracePoints.v2, //End
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);
	
	return HitResult;
}

FTwoVectors UGrabber::GetLineTracePoints() const
{
	FVector StartLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT StartLocation,
		OUT PlayerViewPointRotation
	);
	
	FVector EndLocation = StartLocation + PlayerViewPointRotation.Vector() * Reach;
	
	return FTwoVectors(StartLocation, EndLocation);
}


// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }

	// if physics handle is attached
	if (PhysicsHandle->GrabbedComponent)
	{
		// move the object that is being holded
		PhysicsHandle->SetTargetLocation(GetLineTracePoints().v2);
	}
}

