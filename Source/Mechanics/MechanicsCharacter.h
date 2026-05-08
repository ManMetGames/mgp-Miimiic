#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GrabComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "MechanicsCharacter.generated.h"  // MUST always be the last include

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

// basic first person character
UCLASS(abstract)
class AMechanicsCharacter : public ACharacter
{
    GENERATED_BODY()

    // first person arms mesh (self only)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* FirstPersonMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FirstPersonCameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UGrabComponent* GrabComponent;

    // Input Actions
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* RotateHeldAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* GrabAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* ScrollAction;

protected:
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    class UInputAction* MouseLookAction;

public:
    AMechanicsCharacter();

protected:
    void MoveInput(const FInputActionValue& Value);
    void LookInput(const FInputActionValue& Value);

    // these can be called from blueprint or UI as well as input actions
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoAim(float Yaw, float Pitch);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoMove(float Right, float Forward);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpEnd();

    void StartGrab();
    void StopGrab();
    void StartRotate();
    void StopRotate();
    void ScrollInput(const FInputActionValue& Value);

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:
    USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
    UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};