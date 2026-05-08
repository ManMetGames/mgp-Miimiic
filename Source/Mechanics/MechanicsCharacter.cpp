// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechanicsCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Mechanics.h"

AMechanicsCharacter::AMechanicsCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
    FirstPersonMesh->SetupAttachment(GetMesh());
    FirstPersonMesh->SetOnlyOwnerSee(true);
    FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
    FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
    FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
    FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;
    FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
    FirstPersonCameraComponent->bEnableFirstPersonScale = true;
    FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
    FirstPersonCameraComponent->FirstPersonScale = 0.6f;

    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
    GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
    GetCharacterMovement()->AirControl = 0.5f;

    // grab component handles object pickup, physics handle is found by it in BeginPlay
    GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("GrabComponent"));
    UPhysicsHandleComponent* Handle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
    Handle->InterpolationSpeed = 50.f;
}

void AMechanicsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMechanicsCharacter::DoJumpStart);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMechanicsCharacter::DoJumpEnd);

        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMechanicsCharacter::MoveInput);

        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMechanicsCharacter::LookInput);
        EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMechanicsCharacter::LookInput);

        // hold to grab, release to drop
        EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, &AMechanicsCharacter::StartGrab);
        EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Completed, this, &AMechanicsCharacter::StopGrab);

        EnhancedInputComponent->BindAction(ScrollAction, ETriggerEvent::Triggered, this, &AMechanicsCharacter::ScrollInput);

        // R held redirects mouse into object rotation
        EnhancedInputComponent->BindAction(RotateHeldAction, ETriggerEvent::Started, this, &AMechanicsCharacter::StartRotate);
        EnhancedInputComponent->BindAction(RotateHeldAction, ETriggerEvent::Completed, this, &AMechanicsCharacter::StopRotate);
    }
    else
    {
        UE_LOG(LogMechanics, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
    }
}

void AMechanicsCharacter::MoveInput(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    DoMove(MovementVector.X, MovementVector.Y);
}

void AMechanicsCharacter::LookInput(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    // steal mouse delta for object rotation if R is held
    if (GrabComponent && GrabComponent->IsRotating())
    {
        GrabComponent->AddRotationInput(LookAxisVector.X, LookAxisVector.Y);
    }
    else
    {
        DoAim(LookAxisVector.X, LookAxisVector.Y);
    }
}

void AMechanicsCharacter::DoAim(float Yaw, float Pitch)
{
    if (GetController())
    {
        AddControllerYawInput(Yaw);
        AddControllerPitchInput(Pitch);
    }
}

void AMechanicsCharacter::DoMove(float Right, float Forward)
{
    if (GetController()) {
        AddMovementInput(GetActorRightVector(), Right);
        AddMovementInput(GetActorForwardVector(), Forward);
    }
}

void AMechanicsCharacter::DoJumpStart()
{
    Jump();
}

void AMechanicsCharacter::DoJumpEnd()
{
    StopJumping();
}

void AMechanicsCharacter::StartGrab()
{
    GrabComponent->TryGrab();
}

void AMechanicsCharacter::StopGrab()
{
    GrabComponent->ReleaseGrab();
}

void AMechanicsCharacter::ScrollInput(const FInputActionValue& Value)
{
    float ScrollValue = Value.Get<float>();
    GrabComponent->AdjustGrabDistance(ScrollValue);
}

void AMechanicsCharacter::StartRotate()
{
    GrabComponent->StartRotating();
}

void AMechanicsCharacter::StopRotate()
{
    GrabComponent->StopRotating();
}