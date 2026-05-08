#include "GrabComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/MeshComponent.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentGrabState = EGrabState::Idle;
    HeldActor = nullptr;
}

void UGrabComponent::BeginPlay()
{
    Super::BeginPlay();

    // find components on the owner rather than requiring manual assignment
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
    else {
        FHitResult Hit;

        FVector Start = PlayerCamera->GetComponentLocation();
        FVector End = Start + PlayerCamera->GetForwardVector() * GrabReach;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(GetOwner());
        bool bGotHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

        if (bGotHit)
        {
            UPrimitiveComponent* HitComp = Hit.GetComponent();

            // clear outline if we moved to a different object
            if (HoveredComponent != nullptr && HoveredComponent != HitComp)
            {
                UMeshComponent* OldMesh = Cast<UMeshComponent>(HoveredComponent);
                if (OldMesh != nullptr)
                {
                    OldMesh->SetOverlayMaterial(nullptr);
                }
                HoveredComponent = nullptr;
            }

            if (HitComp != nullptr && HitComp->IsSimulatingPhysics())
            {
                UMeshComponent* NewMesh = Cast<UMeshComponent>(HitComp);
                if (NewMesh != nullptr)
                {
                    NewMesh->SetOverlayMaterial(HoverOutlineMaterial);
                }
                HoveredComponent = HitComp;
                CurrentGrabState = EGrabState::Hovering;
            }
            else
            {
                CurrentGrabState = EGrabState::Idle;
            }
        }
        else
        {
            // nothing in range, clear outline
            if (HoveredComponent != nullptr)
            {
                UMeshComponent* Mesh = Cast<UMeshComponent>(HoveredComponent);
                if (Mesh != nullptr)
                {
                    Mesh->SetOverlayMaterial(nullptr);
                }
                HoveredComponent = nullptr;
            }
            CurrentGrabState = EGrabState::Idle;
        }
    }
}

void UGrabComponent::TryGrab()
{
    if (CurrentGrabState == EGrabState::Grabbed) return;
    if (!PhysicsHandle || !PlayerCamera) return;

    FVector Start = PlayerCamera->GetComponentLocation();
    FVector End = Start + PlayerCamera->GetForwardVector() * GrabReach;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        UPrimitiveComponent* HitComponent = Hit.GetComponent();
        if (HitComponent != nullptr && HitComponent->IsSimulatingPhysics())
        {
            HeldActor = Hit.GetActor();
            // keep rotation so it doesnt snap when picked up
            HeldObjectRotation = HitComponent->GetComponentRotation();

            PhysicsHandle->GrabComponentAtLocationWithRotation(
                HitComponent,
                NAME_None,
                HitComponent->GetComponentLocation(),
                HeldObjectRotation
            );

            ApplyWeightSimulation(HitComponent);

            if (HoveredComponent != nullptr)
            {
                UMeshComponent* Mesh = Cast<UMeshComponent>(HoveredComponent);
                if (Mesh != nullptr)
                {
                    Mesh->SetOverlayMaterial(nullptr);
                }
                HoveredComponent = nullptr;
            }

            CurrentGrabState = EGrabState::Grabbed;
            UE_LOG(LogTemp, Warning, TEXT("grabbed %s"), *HeldActor->GetName());
        }
    }
}


void UGrabComponent::ApplyWeightSimulation(UPrimitiveComponent* GrabbedComponent)
{
    if (PhysicsHandle == nullptr || GrabbedComponent == nullptr) return;

    float Mass = GrabbedComponent->GetMass();

    // t is 0 at light threshold, 1 at heavy
    float t = FMath::GetRangePct(LightMassThreshold, HeavyMassThreshold, Mass);
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;

    float SpeedRange = HeavyInterpolationSpeed - LightInterpolationSpeed;
    PhysicsHandle->InterpolationSpeed = LightInterpolationSpeed + SpeedRange * t;
}

void UGrabComponent::ReleaseGrab()
{
    if (CurrentGrabState != EGrabState::Grabbed) return;
    if (PhysicsHandle == nullptr) return;

    PhysicsHandle->ReleaseComponent();
    PhysicsHandle->InterpolationSpeed = LightInterpolationSpeed;

    HeldActor = nullptr;
    bIsRotating = false;
    HeldObjectRotation = FRotator::ZeroRotator;
    CurrentGrabState = EGrabState::Idle;
}

void UGrabComponent::AdjustGrabDistance(float ScrollValue)
{
    if (CurrentGrabState != EGrabState::Grabbed) return;
    if (bIsRotating) return;

    GrabDistance = GrabDistance + ScrollValue * 50.f;

    if (GrabDistance < MinGrabDistance) GrabDistance = MinGrabDistance;
    if (GrabDistance > MaxGrabDistance) GrabDistance = MaxGrabDistance;
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
    if (CurrentGrabState != EGrabState::Grabbed) return;
    if (!bIsRotating) return;

    HeldObjectRotation.Yaw += YawDelta * RotationSpeed;
    HeldObjectRotation.Pitch += PitchDelta * RotationSpeed;
}

void UGrabComponent::UpdateHeldObject()
{
    if (PhysicsHandle == nullptr || PlayerCamera == nullptr) return;

    FVector TargetLocation = PlayerCamera->GetComponentLocation()
        + PlayerCamera->GetForwardVector() * GrabDistance;

    // use stored rotation when rotating, otherwise follow camera
    FRotator TargetRotation;
    if (bIsRotating)
    {
        TargetRotation = HeldObjectRotation;
    }
    else
    {
        TargetRotation = PlayerCamera->GetComponentRotation();
    }

    PhysicsHandle->SetTargetLocation(TargetLocation);
    PhysicsHandle->SetTargetRotation(TargetRotation);

    // drop it if its stuck
    if (PhysicsHandle->GetGrabbedComponent() != nullptr)
    {
        FVector ActualLocation = PhysicsHandle->GetGrabbedComponent()->GetComponentLocation();
        float Stretch = FVector::Dist(ActualLocation, TargetLocation);

        if (Stretch > MaxHoldStretchDistance)
        {
            UE_LOG(LogTemp, Warning, TEXT("auto drop"));
            ReleaseGrab();
        }
    }
}