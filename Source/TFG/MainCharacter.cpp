// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#include "TFG.h"
#include "MainCharacter.h"
#include "Animation/AnimInstance.h"

#define CapsuleRadius 42.0f
#define CapsuleHalfHeight 96.0f

#define SprintSpeed 800.0f
#define WalkSpeed 250.0f

//////////////////////////////////////////////////////////////////////////
// AMainCharacter

// Sets default values
AMainCharacter::AMainCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight); // Default: 55.0f and 96.0f
	
	// Initialize mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshObj(TEXT("/Game/Hero/Mannequin/Mesh/Hero_Mannequin"));
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimationObj(TEXT("/Game/Hero/Animations/HeroAnimationBlueprint"));
	GetMesh()->SkeletalMesh = SkeletalMeshObj.Object;
	GetMesh()->AnimClass = AnimationObj.Object->GeneratedClass;
	GetMesh()->RelativeLocation = FVector(0.0f, 0.0f, -CapsuleHalfHeight);
	GetMesh()->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = false;
	GetMesh()->bCastDynamicShadow = true;
	GetMesh()->bReceivesDecals = false;

	// First person camera arm
	FirstPersonCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonCameraArm"));
	GetFirstPersonCameraArm()->SetupAttachment(GetMesh(), TEXT("HeadSocket"));
	GetFirstPersonCameraArm()->RelativeRotation = FRotator(0.0f, 90.0f, 0.0f);
	GetFirstPersonCameraArm()->TargetArmLength = -20.0f;

	// First person camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	GetFirstPersonCameraComponent()->SetupAttachment(FirstPersonCameraArm);
	GetFirstPersonCameraComponent()->bUsePawnControlRotation = true;
	GetFirstPersonCameraComponent()->FieldOfView = 95.0f;

	// Set turn rates for pad controller
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Character movement setups
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->AirControl = 0.7f;
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	// Call the base class
	Super::Tick(DeltaTime);

	// Check if can blink in the tick
	CheckCanBlink();

	// Adrenaline management: Recover adrenaline per tick
	if (CurrentAdrenaline < MaximumAdrenaline)
	{
		CurrentAdrenaline++;
	}

	// Adrenaline management: Time dilation
	if (bTimeDilationIsActive)
	{
		// If the current adrenaline isn't enought to keep the bullet time on, we stop it
		// Else, we calculate the new current adrenaline
		if (CurrentAdrenaline < AdrenalinePerTimeDilation)
		{
			EndTimeDilation();
		}
		else
		{
			CurrentAdrenaline = FMath::Max(CurrentAdrenaline - AdrenalinePerTimeDilation, 0.0f);
		}
	}
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Movement
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	// Camera movement
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpAtRate);

	// Action Button
	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AMainCharacter::OnAction);

	// Sprint
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::OnSprintReleased);

	// Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Crouch
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::OnCrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMainCharacter::OnCrouchReleased);

	// Special Power: Time Dilation
	PlayerInputComponent->BindAction("TimeDilation", IE_Pressed, this, &AMainCharacter::OnTimeDilationToggle);

	// Special Power: Blink
	PlayerInputComponent->BindAction("Blink", IE_Pressed, this, &AMainCharacter::OnBlink);
}

void AMainCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// Add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::OnAction()
{
	// TODO
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void AMainCharacter::OnSprintPressed()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AMainCharacter::OnSprintReleased()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMainCharacter::OnCrouchPressed()
{
	Crouch();
}

void AMainCharacter::OnCrouchReleased()
{
	UnCrouch();
}

void AMainCharacter::OnTimeDilationToggle()
{
	// Checks if the player can use powers
	if (bIsPowered)
	{
		// Toggle the time dilation (bullet time)
		if (bTimeDilationIsActive)
		{
			EndTimeDilation();
		}
		else {
			StartTimeDilation();
		}
	}
}

void AMainCharacter::StartTimeDilation()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationPlayerRate);
	this->CustomTimeDilation = TimeDilationPlayerRate;
	bTimeDilationIsActive = true;
}

void AMainCharacter::EndTimeDilation()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	this->CustomTimeDilation = 1.0f;
	bTimeDilationIsActive = false;
}

void AMainCharacter::OnBlink()
{
	if (bCanBlink && CurrentAdrenaline >= AdrenalinePerBlink) {
		bBlinkIsActive = true;
		ACharacter::Jump();
		// Make delay
		this->SetActorLocationAndRotation(BlinkLocation + FVector(0, 0, 100), GetCapsuleComponent()->GetComponentRotation());
		CurrentAdrenaline = FMath::Max(CurrentAdrenaline - AdrenalinePerBlink, 0.0f);
		bBlinkIsActive = false;
	}
}

void AMainCharacter::CheckCanBlink()
{
	if (bIsPowered && !bBlinkIsActive)
	{
		// Trace a line from the camera location in the direction where the player was looking
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
		RV_TraceParams.bTraceComplex = true;
		RV_TraceParams.bTraceAsyncScene = true;
		RV_TraceParams.bReturnPhysicalMaterial = false;

		FHitResult RV_Hit(ForceInit);

		GetWorld()->LineTraceSingleByChannel(
			RV_Hit, // Result
			FirstPersonCameraComponent->GetComponentLocation(), // Start
			FirstPersonCameraComponent->GetComponentLocation() +
			(FirstPersonCameraComponent->GetForwardVector() * BlinkMaximumDistance), // End
			ECC_Visibility, // Collision channel
			RV_TraceParams
		);

		// If was a blocking hit, it return true and update the blink location too
		if (RV_Hit.bBlockingHit)
		{
			BlinkLocation = RV_Hit.ImpactPoint;
			bCanBlink = true;
		}
		else {
			bCanBlink = false;
		}
	}
	else
	{
		bCanBlink = false;
	}
}
