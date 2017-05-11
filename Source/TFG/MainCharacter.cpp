// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#include "TFG.h"
#include "MainCharacter.h"

#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/WidgetBlueprintLibrary.h"

#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

#include "HeroAnimationInstance.h"

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
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshObjectFinder(TEXT("/Game/Hero/Mannequin/Mesh/Hero_Mannequin"));
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimationBlueprintObjectFinder(TEXT("/Game/Hero/Animations/HeroAnimationBlueprint"));
	GetMesh()->SkeletalMesh = SkeletalMeshObjectFinder.Object;
	GetMesh()->AnimClass = AnimationBlueprintObjectFinder.Object->GeneratedClass;
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
	GetFirstPersonCameraComponent()->bAutoActivate = true;

	// Third person camera arm
	ThirdPersonCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonCameraArm"));
	GetThirdPersonCameraArm()->SetupAttachment(GetMesh(), TEXT("HeadSocket"));
	GetThirdPersonCameraArm()->RelativeRotation = FRotator(0.0f, 90.0f, -90.0f);
	GetThirdPersonCameraArm()->TargetArmLength = 200.0f;
	GetThirdPersonCameraArm()->bUsePawnControlRotation = true;

	// Third person camera
	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	GetThirdPersonCameraComponent()->SetupAttachment(ThirdPersonCameraArm);
	//GetThirdPersonCameraComponent()->bUsePawnControlRotation = true;
	GetThirdPersonCameraComponent()->FieldOfView = 95.0f;
	GetThirdPersonCameraComponent()->bAutoActivate = false;

	// Climb detector
	ClimbSurfaceDetector = CreateDefaultSubobject<USphereComponent>(TEXT("ClimbSurfaceDetector"));
	GetClimbSurfaceDetector()->SetupAttachment(RootComponent);
	GetClimbSurfaceDetector()->InitSphereRadius(100.0f);
	GetClimbSurfaceDetector()->bGenerateOverlapEvents = true;
	GetClimbSurfaceDetector()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Block);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	GetClimbSurfaceDetector()->OnComponentBeginOverlap.AddDynamic(this, &AMainCharacter::OnClimbSurfaceDetectorBeginOverlap);
	GetClimbSurfaceDetector()->OnComponentEndOverlap.AddDynamic(this, &AMainCharacter::OnClimbSurfaceDetectorEndOverlap);

	// Set turn rates for pad controller
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Character movement setups
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->CrouchedHalfHeight = 65.0f;
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

	// Trace check
	if (bCanTrace == true)
	{
		DoTrace();
	}
	else
	{
		// TODO
	}
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());

	// HUD initialization
	//static ConstructorHelpers::FClassFinder<UUserWidget> WidgetFinder(TEXT("/Game/Blueprints/HUD/HUDWidget"));
	if (HUDWidget && PlayerController)
	{
		UUserWidget* UserWidget = UWidgetBlueprintLibrary::Create(GetWorld(), HUDWidget, PlayerController);
		UserWidget->AddToViewport();
	}

	// Animation access util
	Animation = Cast<UHeroAnimationInstance>(GetMesh()->GetAnimInstance());
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

	PlayerInputComponent->BindAxis("RotationTrick", this, &AMainCharacter::RotationTrick);

	// Action Button
	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AMainCharacter::OnAction);

	// Sprint
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::OnSprintReleased);

	// Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::OnJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Crouch
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::OnCrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMainCharacter::OnCrouchReleased);

	// Special Power: Time Dilation
	PlayerInputComponent->BindAction("TimeDilation", IE_Pressed, this, &AMainCharacter::OnTimeDilationToggle);

	// Special Power: Blink
	PlayerInputComponent->BindAction("Blink", IE_Pressed, this, &AMainCharacter::OnBlink);

	// Util: Camera toggle
	PlayerInputComponent->BindAction("CameraToggle", IE_Pressed, this, &AMainCharacter::OnCameraToggle);
}

void AMainCharacter::MoveForward(float Value)
{
	
	if (!bIsHanging)
	{
		if (Value != 0.0f)
		{
			// Add movement in that direction
			AddMovementInput(GetActorForwardVector(), Value);
		}
	}
	else
	{
		if (bCanBraceHang)
		{
			if (Value < 0.0f)
			{
				GrabLedgeRear();
			}
			else
			{
				GrabLedgeRearCancel();
			}
		}
	}
}

void AMainCharacter::MoveRight(float Value)
{
	if (!bIsHanging)
	{
		if (Value != 0.0f)
		{
			// Add movement in that direction
			AddMovementInput(GetActorRightVector(), Value);
		}
	}
	else
	{
		if (!bIsGrabbingLookingRear)
		{
			GrabLedgeMove();
		}
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

void AMainCharacter::RotationTrick(float Rate)
{
	Animation->RotationInput = Rate;
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

void AMainCharacter::OnJump()
{
	if (!bIsHanging)
	{
		ACharacter::Jump();
	}
	else
	{
		if (bIsGrabbingLookingRear)
		{
			AMainCharacter::JumpGrabbingRear();
		}
		else
		{
			if (bIsGrabbingLookingSide)
			{
				AMainCharacter::JumpGrabbingSide();
			}
			else
			{
				AMainCharacter::ClimbLedge();
			}
		}
	}
}

void AMainCharacter::OnCrouchPressed()
{
	if(!bIsHanging)
	{
		Crouch();
	}
	else
	{
		LeaveLedge();
	}
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
		FTimerHandle BlinkHandle;
		GetWorldTimerManager().SetTimer(BlinkHandle, this, &AMainCharacter::OnBlinkTimerEnd, 0.25f, false);
	}
}

void AMainCharacter::OnBlinkTimerEnd()
{
	this->SetActorLocationAndRotation(BlinkLocation + FVector(0, 0, 100), GetCapsuleComponent()->GetComponentRotation());
	CurrentAdrenaline = FMath::Max(CurrentAdrenaline - AdrenalinePerBlink, 0.0f);
	bBlinkIsActive = false;
}

void AMainCharacter::CheckCanBlink()
{
	if (bIsPowered && !bBlinkIsActive)
	{
		// Trace a line from the camera location in the direction where the player was looking
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("BlinkTracer")), true, this);
		RV_TraceParams.bTraceComplex = true;
		RV_TraceParams.bTraceAsyncScene = true;
		RV_TraceParams.bReturnPhysicalMaterial = false;

		FHitResult RV_Hit(ForceInit);

		GetWorld()->LineTraceSingleByChannel(
			RV_Hit, // Result
			FirstPersonCameraComponent->GetComponentLocation(), // Start
			FirstPersonCameraComponent->GetComponentLocation() +
			(FirstPersonCameraComponent->GetForwardVector() * BlinkMaximumDistance), // End
			ECollisionChannel::ECC_Visibility, // Collision channel
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

void AMainCharacter::OnCameraToggle()
{
	GetFirstPersonCameraComponent()->ToggleActive();
	GetThirdPersonCameraComponent()->ToggleActive();
}

void AMainCharacter::OnClimbSurfaceDetectorBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	bCanTrace = true;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "Sphere can trace");
}

void AMainCharacter::OnClimbSurfaceDetectorEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	bCanTrace = false;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "Sphere cannot trace");
}

void AMainCharacter::DoTrace()
{
	if(GetCharacterMovement()->IsFalling() && !bIsHanging) // TODO
	{
		// Wall tracer
		FCollisionQueryParams RV_TraceParams_Wall = FCollisionQueryParams(FName(TEXT("WallTracer")), true, this);
		RV_TraceParams_Wall.bTraceComplex = true;
		RV_TraceParams_Wall.bTraceAsyncScene = true;
		RV_TraceParams_Wall.bReturnPhysicalMaterial = false;

		FHitResult RV_Hit_Wall(ForceInit);

		GetWorld()->SweepSingleByChannel(
			RV_Hit_Wall, // Result
			GetActorLocation(), // Start
			GetActorLocation() + (GetActorForwardVector() * FVector(150.0f, 150.0f, 0)), // End
			FQuat(),
			ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
			FCollisionShape::MakeSphere(20.0f), // Radius
			RV_TraceParams_Wall
		);
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("WallTracer"));

		if (RV_Hit_Wall.bBlockingHit)
		{
			WallImpact = RV_Hit_Wall.ImpactPoint;
			WallNormal = RV_Hit_Wall.ImpactNormal;

			// Roof tracer
			FCollisionQueryParams RV_TraceParams_Roof = FCollisionQueryParams(FName(TEXT("RoofTracer")), true, this);
			RV_TraceParams_Roof.bTraceComplex = true;
			RV_TraceParams_Roof.bTraceAsyncScene = true;
			RV_TraceParams_Roof.bReturnPhysicalMaterial = false;

			FHitResult RV_Hit_Roof(ForceInit);

			GetWorld()->SweepSingleByChannel(
				RV_Hit_Roof, // Result
				GetActorLocation() + (GetActorForwardVector() * 0.95f), // Start
				GetActorLocation() + (GetActorForwardVector() * 0.95f) + FVector(0, 0, 150.0f), // End
				FQuat(),
				ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
				FCollisionShape::MakeSphere(20.0f), // Radius
				RV_TraceParams_Roof
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("RoofTracer"));

			if (RV_Hit_Roof.bBlockingHit)
			{
				// TODO
			}
		}

		// Height tracer: Left
		FCollisionQueryParams RV_TraceParams_Height_Left = FCollisionQueryParams(FName(TEXT("HeightTracerLeft")), true, this);
		RV_TraceParams_Height_Left.bTraceComplex = true;
		RV_TraceParams_Height_Left.bTraceAsyncScene = true;
		RV_TraceParams_Height_Left.bReturnPhysicalMaterial = false;

		FHitResult RV_Hit_Height_Left(ForceInit);

		GetWorld()->SweepSingleByChannel(
			RV_Hit_Height_Left, // Result
			GetActorLocation() + FVector(0, 0, 600.0f) + (GetActorForwardVector() * 55.0f) - (GetActorRightVector() * 35.0f), // Start
			GetActorLocation() + FVector(0, 0, 600.0f) + (GetActorForwardVector() * 55.0f) - (GetActorRightVector() * 35.0f) - FVector(0, 0, 550.0f), // End
			FQuat(),
			ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
			FCollisionShape::MakeSphere(10.0f), // Radius
			RV_TraceParams_Height_Left
		);
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("HeightTracerLeft"));

		// Height tracer: Right
		FCollisionQueryParams RV_TraceParams_Height_Right = FCollisionQueryParams(FName(TEXT("HeightTracerRight")), true, this);
		RV_TraceParams_Height_Right.bTraceComplex = true;
		RV_TraceParams_Height_Right.bTraceAsyncScene = true;
		RV_TraceParams_Height_Right.bReturnPhysicalMaterial = false;

		FHitResult RV_Hit_Height_Right(ForceInit);

		GetWorld()->SweepSingleByChannel(
			RV_Hit_Height_Right, // Result
			GetActorLocation() + FVector(0, 0, 600.0f) + (GetActorForwardVector() * 55.0f) + (GetActorRightVector() * 35.0f), // Start
			GetActorLocation() + FVector(0, 0, 600.0f) + (GetActorForwardVector() * 55.0f) + (GetActorRightVector() * 35.0f) - FVector(0, 0, 550.0f), // End
			FQuat(),
			ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
			FCollisionShape::MakeSphere(10.0f), // Radius
			RV_TraceParams_Height_Right
		);
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("HeightTracerRight"));

		if (RV_Hit_Height_Left.bBlockingHit && RV_Hit_Height_Right.bBlockingHit)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "Height tracer succeed");
			LedgeHeight = RV_Hit_Height_Left.ImpactPoint; // We take the left impact point, at the moment

			// Climbing checks - Good location?????????????????????????????????????????????????????????????????????????????????
			if (true) // TODO
			{
				if ((((GetMesh()->GetSocketLocation(TEXT("HipSocket"))).Z - (LedgeHeight.Z)) > -70.0f)
					&& (((GetMesh()->GetSocketLocation(TEXT("HipSocket"))).Z - (LedgeHeight.Z)) < 0.0f))
				{
					GrabLedge();
				}
				else
				{
					ResetGrabLedge();
					KneeClimbLedge();
				}
			}
		}
	}
	else
	{
		if (bIsHanging)
		{
			// Foot tracer: Left
			FCollisionQueryParams RV_TraceParams_Foot_Left = FCollisionQueryParams(FName(TEXT("FootTracerLeft")), true, this);
			RV_TraceParams_Foot_Left.bTraceComplex = true;
			RV_TraceParams_Foot_Left.bTraceAsyncScene = true;
			RV_TraceParams_Foot_Left.bReturnPhysicalMaterial = false;

			FHitResult RV_Hit_Foot_Left(ForceInit);

			GetWorld()->SweepSingleByChannel(
				RV_Hit_Foot_Left, // Result
				GetActorLocation() + FVector(0, 0, -50.0f) - (GetActorRightVector() * 35.0f), // Start
				GetActorLocation() + FVector(0, 0, -50.0f) + (GetActorForwardVector() * FVector(50.0f, 50.0f, 0)) - (GetActorRightVector() * 35.0f), // End
				FQuat(),
				ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
				FCollisionShape::MakeSphere(10.0f), // Radius
				RV_TraceParams_Foot_Left
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("FootTracerLeft"));

			// Foot tracer: Right
			FCollisionQueryParams RV_TraceParams_Foot_Right = FCollisionQueryParams(FName(TEXT("FootTracerRight")), true, this);
			RV_TraceParams_Foot_Right.bTraceComplex = true;
			RV_TraceParams_Foot_Right.bTraceAsyncScene = true;
			RV_TraceParams_Foot_Right.bReturnPhysicalMaterial = false;

			FHitResult RV_Hit_Foot_Right(ForceInit);

			GetWorld()->SweepSingleByChannel(
				RV_Hit_Foot_Right, // Result
				GetActorLocation() + FVector(0, 0, -50.0f) + (GetActorRightVector() * 35.0f), // Start
				GetActorLocation() + FVector(0, 0, -50.0f) + (GetActorForwardVector() * FVector(50.0f, 50.0f, 0)) + (GetActorRightVector() * 35.0f), // End
				FQuat(),
				ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
				FCollisionShape::MakeSphere(10.0f), // Radius
				RV_TraceParams_Foot_Right
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("FootTracerRight"));

			// If the two traces hit, the character can keep their feet close to the wall (braced hanging)
			if (RV_Hit_Foot_Left.bBlockingHit && RV_Hit_Foot_Right.bBlockingHit)
			{
				bCanBraceHang = true;
				Animation->bCanBraceHang = true;
			}
			else
			{
				bCanBraceHang = false;
				Animation->bCanBraceHang = false;
			}
		}
	}
}

void AMainCharacter::GrabLedge()
{
	if (bGrabLedgeDoOnce)
	{
		if (!bIsClimbingLedge)
		{
			Animation->bIsHanging = true;

			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			bIsHanging = true;
			bUseControllerRotationYaw = false;

			FLatentActionInfo GrabLedgeMoveLatentInfo;
			GrabLedgeMoveLatentInfo.CallbackTarget = this;
			UKismetSystemLibrary::MoveComponentTo(RootComponent, 
				FVector(
					(WallNormal * FVector(CapsuleRadius, CapsuleRadius, CapsuleRadius)).X + WallImpact.X,
					(WallNormal * FVector(CapsuleRadius, CapsuleRadius, CapsuleRadius)).Y + WallImpact.Y,
					LedgeHeight.Z - 116.0f), 
				AlignToWall(), 
				false, // Ease Out
				false, // Ease In
				0.2f, // Over Time
				true, // Force Rotation Shortest Path
				EMoveComponentAction::Move, // Movement Type
				GrabLedgeMoveLatentInfo);

			GetCharacterMovement()->StopMovementImmediately();
			
			// TODO
		}

		bGrabLedgeDoOnce = false;
	}
}

void AMainCharacter::ResetGrabLedge()
{
	bGrabLedgeDoOnce = true;
}

void AMainCharacter::LeaveLedge()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	Animation->bIsHanging = false;
	Animation->bCanBraceHang = false;
	Animation->GrabRightInput = 0.0f;
	Animation->GrabLookingRightInput = 0.0f;
	Animation->bIsGrabingAndLookingRear = false;
	bIsHanging = false;
	bCanBraceHang = false;
	bIsGrabbingLookingSide = false;
	bIsGrabbingLookingRear = false;
	bUseControllerRotationYaw = true;
	// TODO
}

void AMainCharacter::ClimbLedge()
{
	Animation->bIsHanging = false;
	Animation->bCanBraceHang = false;
	Animation->GrabRightInput = 0.0f;
	Animation->GrabLookingRightInput = 0.0f;
	Animation->bIsGrabingAndLookingRear = false;
	bIsHanging = false;
	bCanBraceHang = false;
	bIsGrabbingLookingSide = false;
	bIsGrabbingLookingRear = false;
}

void AMainCharacter::KneeClimbLedge()
{

}

void AMainCharacter::GrabLedgeMove()
{
	if (GetInputAxisValue(TEXT("MoveRight")) == 0.0f) // TODO
	{
		GetCharacterMovement()->StopMovementImmediately();
		Animation->GrabRightInput = 0.0f;
		Animation->GrabLookingRightInput = 0.0f;
	}
	else
	{
		if (GetInputAxisValue(TEXT("MoveRight")) > 0)
		{
			// Grab tracer: Right
			FCollisionQueryParams RV_TraceParams_Grab_Right = FCollisionQueryParams(FName(TEXT("GrabTracerRight")), true, this);
			RV_TraceParams_Grab_Right.bTraceComplex = true;
			RV_TraceParams_Grab_Right.bTraceAsyncScene = true;
			RV_TraceParams_Grab_Right.bReturnPhysicalMaterial = false;

			FHitResult RV_Hit_Grab_Right(ForceInit);

			GetWorld()->SweepSingleByChannel(
				RV_Hit_Grab_Right, // Result
				GetActorLocation() + FVector(0, 0, 110.0f) + (GetActorForwardVector() * 50.0f) + (GetActorRightVector() * 50.0f), // Start
				GetActorLocation() + FVector(0, 0, 85.0f) + (GetActorForwardVector() * 50.0f) + (GetActorRightVector() * 50.0f), // End
				FQuat(),
				ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
				FCollisionShape::MakeSphere(10.0f), // Radius
				RV_TraceParams_Grab_Right
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("GrabTracerRight"));

			if (RV_Hit_Grab_Right.bBlockingHit)
			{
				Animation->GrabRightInput = GetInputAxisValue(TEXT("MoveRight"));
				Animation->GrabLookingRightInput = 0.0f;
				if (bCanBraceHang)
				{
					AddMovementInput(GetActorRightVector(), GetInputAxisValue(TEXT("MoveRight")) / 5);
				}
				else
				{
					AddMovementInput(GetActorRightVector(), GetInputAxisValue(TEXT("MoveRight")) / 8);
				}
				bIsGrabbingLookingSide = false;
			}
			else
			{
				GetMovementComponent()->StopMovementImmediately();

				// Grab look tracer: Right
				FCollisionQueryParams RV_TraceParams_Grab_Look_Right = FCollisionQueryParams(FName(TEXT("GrabLookTracerRight")), true, this);
				RV_TraceParams_Grab_Look_Right.bTraceComplex = true;
				RV_TraceParams_Grab_Look_Right.bTraceAsyncScene = true;
				RV_TraceParams_Grab_Look_Right.bReturnPhysicalMaterial = false;

				FHitResult RV_Hit_Grab_Look_Right(ForceInit);

				GetWorld()->SweepSingleByChannel(
					RV_Hit_Grab_Look_Right, // Result
					GetActorLocation() + FVector(0, 0, 110.0f) + (GetActorForwardVector() * 50.0f) + (GetActorRightVector() * FVector(50.0f, 50.0f, 0.0f)), // Start
					GetActorLocation() + FVector(0, 0, 110.0f) + (GetActorForwardVector() * 50.0f) + (GetActorRightVector() * 350.0f), // End
					FQuat(),
					ECollisionChannel::ECC_Visibility, // Collision channel
					FCollisionShape::MakeSphere(10.0f), // Radius
					RV_TraceParams_Grab_Look_Right
				);
				//GetWorld()->DebugDrawTraceTag = FName(TEXT("GrabLookTracerRight"));

				if (RV_Hit_Grab_Look_Right.bBlockingHit)
				{
					Animation->GrabRightInput = 0.0f;
					Animation->GrabLookingRightInput = 0.0f;
					bIsGrabbingLookingSide = false;
				}
				else
				{
					Animation->GrabRightInput = 0.0f;
					Animation->GrabLookingRightInput = GetInputAxisValue(TEXT("MoveRight"));
					bIsGrabbingLookingSide = true;
				}
			}
		}
		else
		{
			// Grab tracer: Left
			FCollisionQueryParams RV_TraceParams_Grab_Left = FCollisionQueryParams(FName(TEXT("GrabTracerLeft")), true, this);
			RV_TraceParams_Grab_Left.bTraceComplex = true;
			RV_TraceParams_Grab_Left.bTraceAsyncScene = true;
			RV_TraceParams_Grab_Left.bReturnPhysicalMaterial = false;

			FHitResult RV_Hit_Grab_Left(ForceInit);

			GetWorld()->SweepSingleByChannel(
				RV_Hit_Grab_Left, // Result
				GetActorLocation() + FVector(0, 0, 110.0f) + (GetActorForwardVector() * 50.0f) - (GetActorRightVector() * 50.0f), // Start
				GetActorLocation() + FVector(0, 0, 85.0f) + (GetActorForwardVector() * 50.0f) - (GetActorRightVector() * 50.0f), // End
				FQuat(),
				ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
				FCollisionShape::MakeSphere(10.0f), // Radius
				RV_TraceParams_Grab_Left
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("GrabTracerLeft"));

			if (RV_Hit_Grab_Left.bBlockingHit)
			{
				Animation->GrabRightInput = GetInputAxisValue(TEXT("MoveRight"));
				Animation->GrabLookingRightInput = 0.0f;
				if (bCanBraceHang)
				{
					AddMovementInput(GetActorRightVector(), GetInputAxisValue(TEXT("MoveRight")) / 5);
				}
				else
				{
					AddMovementInput(GetActorRightVector(), GetInputAxisValue(TEXT("MoveRight")) / 8);
				}
				bIsGrabbingLookingSide = false;
			}
			else
			{
				GetMovementComponent()->StopMovementImmediately();

				// Grab look tracer: Left
				FCollisionQueryParams RV_TraceParams_Grab_Look_Left = FCollisionQueryParams(FName(TEXT("GrabLookTracerLeft")), true, this);
				RV_TraceParams_Grab_Look_Left.bTraceComplex = true;
				RV_TraceParams_Grab_Look_Left.bTraceAsyncScene = true;
				RV_TraceParams_Grab_Look_Left.bReturnPhysicalMaterial = false;

				FHitResult RV_Hit_Grab_Look_Left(ForceInit);

				GetWorld()->SweepSingleByChannel(
					RV_Hit_Grab_Look_Left, // Result
					GetActorLocation() + FVector(0, 0, 110.0f) + (GetActorForwardVector() * 50.0f) - (GetActorRightVector() * FVector(50.0f, 50.0f, 0.0f)), // Start
					GetActorLocation() + FVector(0, 0, 110.0f) + (GetActorForwardVector() * 50.0f) - (GetActorRightVector() * 350.0f), // End
					FQuat(),
					ECollisionChannel::ECC_Visibility, // Collision channel
					FCollisionShape::MakeSphere(10.0f), // Radius
					RV_TraceParams_Grab_Look_Left
				);
				//GetWorld()->DebugDrawTraceTag = FName(TEXT("GrabLookTracerLeft"));

				if (RV_Hit_Grab_Look_Left.bBlockingHit)
				{
					Animation->GrabRightInput = 0.0f;
					Animation->GrabLookingRightInput = 0.0f;
					bIsGrabbingLookingSide = false;
				}
				else
				{
					Animation->GrabRightInput = 0.0f;
					Animation->GrabLookingRightInput = GetInputAxisValue(TEXT("MoveRight"));
					bIsGrabbingLookingSide = true;
				}
			}
		}
	}
}

void AMainCharacter::GrabLedgeRear()
{
	if (!bIsGrabbingLookingRear)
	{
		bIsGrabbingLookingRear = true;
		Animation->bIsGrabingAndLookingRear = true;
		// TODO: Camera restrictions
	}
}

void AMainCharacter::GrabLedgeRearCancel()
{
	if(bIsGrabbingLookingRear)
	{
		bIsGrabbingLookingRear = false;
		Animation->bIsGrabingAndLookingRear = false;
		// TODO: Camera restrictions
	}
}

void AMainCharacter::JumpGrabbingSide()
{
	LeaveLedge();
	if (GetInputAxisValue(TEXT("MoveRight")) > 0.0f)
	{
		LaunchCharacter((GetActorRightVector() * 500.0f) + FVector(0, 0, 600.0f), false, false);
	}
	else
	{
		if (GetInputAxisValue(TEXT("MoveRight")) < 0.0f)
		{
			LaunchCharacter((UKismetMathLibrary::NegateVector(GetActorRightVector()) * 500.0f) + FVector(0, 0, 600.0f), false, false);
		}
	}
	// TODO: Camera restrictions
}

void AMainCharacter::JumpGrabbingRear()
{
	LeaveLedge();
	LaunchCharacter((WallNormal * 500) + FVector(0, 0, 300.0f), false, false);
	PlayerController->SetControlRotation(GetActorRotation() + FRotator(0, 180.0f, 0));
	// TODO: Camera restrictions
}

FRotator AMainCharacter::AlignToWall()
{
	return UKismetMathLibrary::MakeRotFromXZ((WallNormal * FVector(-1, -1, 0)), GetCapsuleComponent()->GetUpVector());
}

//////////////////////////////////////////////////////////////////////////
// Interface

void AMainCharacter::HUDInterface_Implementation(bool &bCanBlink_Interface, float &CurrentAdrenaline_Interface, float &MaximumAdrenaline_Interface)
{
	bCanBlink_Interface = bCanBlink;
	CurrentAdrenaline_Interface = CurrentAdrenaline;
	MaximumAdrenaline_Interface = MaximumAdrenaline;
}
