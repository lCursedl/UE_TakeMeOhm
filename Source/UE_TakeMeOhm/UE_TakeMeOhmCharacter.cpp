// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_TakeMeOhmCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include <Components/BoxComponent.h>

//////////////////////////////////////////////////////////////////////////
// AUE_TakeMeOhmCharacter

AUE_TakeMeOhmCharacter::AUE_TakeMeOhmCharacter() {
  IsAlive = true;
  Interval = 0.0f;
  StompTime = 0.0f;
  StompedDuration = 2.0f;
  SkeletalMesh = GetMesh();
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 65.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.75f;

  //Create a box collider for stomp detection
  HeadCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadCollider"));
  HeadCollider->SetupAttachment(RootComponent);

	// Create a camera boom (pulls in towards the player if there is a collision)
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	//CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUE_TakeMeOhmCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AUE_TakeMeOhmCharacter::ConditionJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUE_TakeMeOhmCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUE_TakeMeOhmCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	/*PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AUE_TakeMeOhmCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AUE_TakeMeOhmCharacter::LookUpAtRate);*/

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AUE_TakeMeOhmCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AUE_TakeMeOhmCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AUE_TakeMeOhmCharacter::OnResetVR);
}

void
AUE_TakeMeOhmCharacter::Tick(float DeltaTime) {
  if (IsAlive) {
    if (Stomped) {
      StompTime += DeltaTime;
      if (StompTime >= StompedDuration) {
        StompTime = 0.0f;
        Interval = 0.0f;
        Stomped = false;
        SkeletalMesh->SetVisibility(true);
        HeadCollider->Activate();
      }
      else {
        Interval += DeltaTime;
        if (Interval >= FlickerInterval) {
          SkeletalMesh->SetVisibility(!SkeletalMesh->IsVisible());
          Interval = 0.0f;
        }
      }
    }
  }  
}

void
AUE_TakeMeOhmCharacter::SetStomped(bool value) {
  Stomped = value;
}


void AUE_TakeMeOhmCharacter::OnResetVR()
{
	// If UE_TakeMeOhm is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in UE_TakeMeOhm.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AUE_TakeMeOhmCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AUE_TakeMeOhmCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AUE_TakeMeOhmCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUE_TakeMeOhmCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AUE_TakeMeOhmCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AUE_TakeMeOhmCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void
AUE_TakeMeOhmCharacter::ConditionJump() {
  if (!Stomped) {
    Jump();
  }
}
