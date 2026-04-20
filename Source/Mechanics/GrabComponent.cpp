#include "GrabComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentGrabState = EGrabState::Idle;
    HeldActor = nullptr;
}

void UGrabComponent::BeginPlay()
{
    Super::BeginPlay();

    PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
    if (!PhysicsHandle)
    {
        UE_LOG(LogTemp, Error, TEXT("GrabComponent: No PhysicsHandleComponent found on %s!"), *GetOwner()->GetName());
    }

    PlayerCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!PlayerCamera)
    {
        UE_LOG(LogTemp, Error, TEXT("GrabComponent: No CameraComponent found on %s!"), *GetOwner()->GetName());
    }
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentGrabState == EGrabState::Grabbed)
    {
        UpdateHeldObject();
    }
    else
    {
        FHitResult Hit;
        if (DoLineTrace(Hit))
        {
            CurrentGrabState = EGrabState::Hovering;
        }
        else
        {
            CurrentGrabState = EGrabState::Idle;
        }
    }
}

void UGrabComponent::TryGrab()
{
    if (CurrentGrabState == EGrabState::Grabbed) return;
    if (!PhysicsHandle || !PlayerCamera) return;

    FHitResult Hit;
    if (DoLineTrace(Hit))
    {
        UPrimitiveComponent* HitComponent = Hit.GetComponent();
        if (HitComponent && HitComponent->IsSimulatingPhysics())
        {
            HeldActor = Hit.GetActor();

            // Capture the object's current rotation so it doesn't snap on pickup
            HeldObjectRotation = HitComponent->GetComponentRotation();

            PhysicsHandle->GrabComponentAtLocationWithRotation(
                HitComponent,
                NAME_None,
                HitComponent->GetComponentLocation(),
                HeldObjectRotation
            );

            CurrentGrabState = EGrabState::Grabbed;
            UE_LOG(LogTemp, Log, TEXT("GrabComponent: Grabbed %s"), *HeldActor->GetName());
        }
    }
}

void UGrabComponent::ReleaseGrab()
{
    if (CurrentGrabState != EGrabState::Grabbed) return;
    if (!PhysicsHandle) return;

    PhysicsHandle->ReleaseComponent();
    HeldActor = nullptr;
    bIsRotating = false;
    HeldObjectRotation = FRotator::ZeroRotator;
    CurrentGrabState = EGrabState::Idle;

    UE_LOG(LogTemp, Log, TEXT("GrabComponent: Released object"));
}

void UGrabComponent::AdjustGrabDistance(float ScrollValue)
{
    // Scroll is blocked while rotating so the two inputs don't conflict
    if (CurrentGrabState != EGrabState::Grabbed) return;
    if (bIsRotating) return;

    GrabDistance = FMath::Clamp(
        GrabDistance + ScrollValue * 50.f,
        MinGrabDistance,
        MaxGrabDistance
    );
}

void UGrabComponent::StartRotating()
{
    if (CurrentGrabState != EGrabState::Grabbed) return;
    bIsRotating = true;
}

void UGrabComponent::StopRotating()
{
    bIsRotating = false;
}

void UGrabComponent::AddRotationInput(float YawDelta, float PitchDelta)
{
    if (!bIsRotating) return;
    if (CurrentGrabState != EGrabState::Grabbed) return;

    // Yaw rotates around world Z, Pitch rotates around world Y
    HeldObjectRotation.Yaw += YawDelta * RotationSpeed;
    HeldObjectRotation.Pitch += PitchDelta * RotationSpeed;
}

bool UGrabComponent::DoLineTrace(FHitResult& OutHit)
{
    if (!PlayerCamera) return false;

    FVector Start = PlayerCamera->GetComponentLocation();
    FVector End = Start + PlayerCamera->GetForwardVector() * GrabReach;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void UGrabComponent::UpdateHeldObject()
{
    if (!PhysicsHandle || !PlayerCamera) return;

    FVector TargetLocation = PlayerCamera->GetComponentLocation()
        + PlayerCamera->GetForwardVector() * GrabDistance;

    // When rotating: use accumulated HeldObjectRotation
    // When idle:     follow the camera so the object feels "locked" to the view
    FRotator TargetRotation = bIsRotating
        ? HeldObjectRotation
        : PlayerCamera->GetComponentRotation();

    PhysicsHandle->SetTargetLocation(TargetLocation);
    PhysicsHandle->SetTargetRotation(TargetRotation);
}