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
    EGrabState GetGrabState() const { return CurrentGrabState; }

private:
    bool DoLineTrace(FHitResult& OutHit);
    void UpdateHeldObject();

    UPROPERTY()
    UPhysicsHandleComponent* PhysicsHandle;

    UPROPERTY(VisibleAnywhere, Category = "Grab|State")
    EGrabState CurrentGrabState;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float GrabDistance = 250.f;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float MinGrabDistance = 100.f;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float MaxGrabDistance = 500.f;

    UPROPERTY(EditAnywhere, Category = "Grab|Settings")
    float GrabReach = 500.f;

    UPROPERTY()
    AActor* HeldActor;

    UPROPERTY()
    UCameraComponent* PlayerCamera;
};