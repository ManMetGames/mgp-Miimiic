#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Camera/CameraComponent.h"
#include "GrabComponent.generated.h"

UENUM(BlueprintType)
enum class EGrabState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hovering    UMETA(DisplayName = "Hovering"),
    Grabbed     UMETA(DisplayName = "Grabbed")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MECHANICS_API UGrabComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGrabComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void TryGrab();

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void ReleaseGrab();

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void AdjustGrabDistance(float ScrollValue);

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void StartRotating();

    UFUNCTION(BlueprintCallable, Category = "Grab")
    void StopRotating();

    // Called every frame while R is held — pass in mouse X/Y delta
    UFUNCTION(BlueprintCallable, Category = "Grab")
    void AddRotationInput(float YawDelta, float PitchDelta);

    UFUNCTION(BlueprintCallable, Category = "Grab")
    EGrabState GetGrabState() const { return CurrentGrabState; }

    UFUNCTION(BlueprintCallable, Category = "Grab")
    bool IsRotating() const { return bIsRotating; }

private:
    bool DoLineTrace(FHitResult& OutHit);
    void UpdateHeldObject();

    // Applies an interpolation speed to the physics handle based on the grabbed object's mass
    void ApplyWeightSimulation(UPrimitiveComponent* GrabbedComponent);

    UPROPERTY()
    UPhysicsHandleComponent* PhysicsHandle;

    UPROPERTY()
    UCameraComponent* PlayerCamera;

    UPROPERTY(VisibleAnywhere, Category = "Grab|State")
    EGrabState CurrentGrabState;

    // Whether the player is currently holding R to rotate
    UPROPERTY(VisibleAnywhere, Category = "Grab|State")
    bool bIsRotating = false;

    // Accumulated world-space rotation of the held object (independent of camera)
    UPROPERTY(VisibleAnywhere, Category = "Grab|State")
    FRotator HeldObjectRotation = FRotator::ZeroRotator;

    // Tracks whichever component is currently outlined so we can clear it cleanly
    UPROPERTY()
    UPrimitiveComponent* HoveredComponent = nullptr;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float GrabDistance = 250.f;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float MinGrabDistance = 100.f;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float MaxGrabDistance = 500.f;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float GrabReach = 500.f;

    // How fast the object rotates per unit of mouse delta
    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float RotationSpeed = 5.f;

    // If the held object drifts further than this from its target (e.g. stuck in a wall), auto-drop it
    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float MaxHoldStretchDistance = 150.f;

    // The outline material to display on hoverable objects
    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    UMaterialInterface* HoverOutlineMaterial = nullptr;

    // Objects below this mass (kg) are considered light and respond quickly
    UPROPERTY(EditAnywhere, Category = "Grab|Weight")
    float LightMassThreshold = 20.f;

    // Objects above this mass (kg) are considered heavy and respond sluggishly
    UPROPERTY(EditAnywhere, Category = "Grab|Weight")
    float HeavyMassThreshold = 60.f;

    // Physics handle interpolation speed for light objects — snappy response
    UPROPERTY(EditAnywhere, Category = "Grab|Weight")
    float LightInterpolationSpeed = 50.f;

    // Physics handle interpolation speed for heavy objects — sluggish response
    UPROPERTY(EditAnywhere, Category = "Grab|Weight")
    float HeavyInterpolationSpeed = 10.f;

    UPROPERTY()
    AActor* HeldActor;
};