// Copyright (C) 2017 - Fernando Jos� Garc�a Padilla - Universidad de Sevilla

#include "TFG.h"
#include "MainCharacter.h"

#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/WidgetBlueprintLibrary.h"

#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

#include "MainSaveGame.h"
#include "Kismet/GameplayStatics.h"

#include "HeroAnimationInstance.h"

#define CapsuleRadius 42.0f
#define CapsuleHalfHeight 96.0f

#define SprintSpeed 800.0f
#define WalkSpeed 250.0f

#define DefaultViewYawMin 0.0f;
#define DefaultViewYawMax 359.999f;
#define DefaultViewPitchMin -80.0f;
#define DefaultViewPitchMax 70.0f

#define DefaultFPCameraProbeSize 20.0f;
#define HangingFPCameraProbeSize 12.0f;

//////////////////////////////////////////////////////////////////////////
// AMainCharacter

// Sets default values
AMainCharacter::AMainCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight); // Default: 55.0f and 96.0f
	
	// Initialize mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshObjectFinder(TEXT("/Game/Hero/Mannequin/Mesh/Hero_Mannequin"));
	static ConstructorHelpers::FObjectFinder<UClass> AnimationBlueprintObjectFinder(TEXT("/Game/Hero/Animations/HeroAnimationBlueprint.HeroAnimationBlueprint_C"));
	GetMesh()->SkeletalMesh = SkeletalMeshObjectFinder.Object;
	GetMesh()->SetAnimInstanceClass(AnimationBlueprintObjectFinder.Object);
	GetMesh()->RelativeLocation = FVector(-7.5f, 0.0f, -CapsuleHalfHeight);
	GetMesh()->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = false;
	GetMesh()->bCastDynamicShadow = true;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionProfileName(FName("CharacterMesh"));
	//GetMesh()->SetMassScale(TEXT("root"), 80.0f);

	// First person camera arm
	FirstPersonCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonCameraArm"));
	GetFirstPersonCameraArm()->SetupAttachment(GetMesh(), TEXT("HeadSocket"));
	GetFirstPersonCameraArm()->RelativeLocation = FVector(5.0f, 7.5f, 0.0f);
	GetFirstPersonCameraArm()->RelativeRotation = FRotator(0.0f, 90.0f, -90.0f);
	GetFirstPersonCameraArm()->TargetArmLength = -15.0f;
	GetFirstPersonCameraArm()->bDoCollisionTest = true;
	GetFirstPersonCameraArm()->ProbeChannel = ECollisionChannel::ECC_Camera;
	GetFirstPersonCameraArm()->ProbeSize = DefaultFPCameraProbeSize;

	// First person camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	GetFirstPersonCameraComponent()->SetupAttachment(FirstPersonCameraArm);
	GetFirstPersonCameraComponent()->bUsePawnControlRotation = true;
	GetFirstPersonCameraComponent()->FieldOfView = 95.0f;
	GetFirstPersonCameraComponent()->bAutoActivate = true;

	// Third person forward-backward camera arm
	ThirdPersonForwardBackwardCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonForwardBackwardCameraArm"));
	GetThirdPersonForwardBackwardCameraArm()->SetupAttachment(GetMesh(), TEXT("HeadSocket"));
	GetThirdPersonForwardBackwardCameraArm()->RelativeLocation = FVector(5.0f, -25.0f, 0.0f);
	GetThirdPersonForwardBackwardCameraArm()->RelativeRotation = FRotator(0.0f, 0.0f, 90.0f);
	GetThirdPersonForwardBackwardCameraArm()->TargetArmLength = 200.0f;
	GetThirdPersonForwardBackwardCameraArm()->bDoCollisionTest = true;
	GetThirdPersonForwardBackwardCameraArm()->ProbeChannel = ECollisionChannel::ECC_Camera;
	GetThirdPersonForwardBackwardCameraArm()->ProbeSize = 25.0f;
	GetThirdPersonForwardBackwardCameraArm()->bUsePawnControlRotation = true;

	// Third person left-right camera arm
	ThirdPersonLeftRightCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonLeftRightCameraArm"));
	GetThirdPersonLeftRightCameraArm()->SetupAttachment(ThirdPersonForwardBackwardCameraArm);
	GetThirdPersonLeftRightCameraArm()->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
	GetThirdPersonLeftRightCameraArm()->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	GetThirdPersonLeftRightCameraArm()->TargetArmLength = 35.0f;
	GetThirdPersonLeftRightCameraArm()->bDoCollisionTest = true;
	GetThirdPersonLeftRightCameraArm()->ProbeChannel = ECollisionChannel::ECC_Camera;
	GetThirdPersonLeftRightCameraArm()->ProbeSize = 12.0f;
	GetThirdPersonLeftRightCameraArm()->bUsePawnControlRotation = false;

	// Third person camera
	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	GetThirdPersonCameraComponent()->SetupAttachment(ThirdPersonLeftRightCameraArm);
	GetThirdPersonCameraComponent()->RelativeRotation = FRotator(0.0f, 90.0f, 0.0f);
	GetThirdPersonCameraComponent()->FieldOfView = 95.0f;
	GetThirdPersonCameraComponent()->bAutoActivate = false;

	// Dead camera arm
	DeathCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("DeathCameraArm"));
	GetDeathCameraArm()->SetupAttachment(GetCapsuleComponent());
	GetDeathCameraArm()->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	GetDeathCameraArm()->TargetArmLength = 150.0f;
	GetDeathCameraArm()->bDoCollisionTest = true;
	GetDeathCameraArm()->ProbeSize = DefaultFPCameraProbeSize;

	// Dead camera
	DeathCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	GetDeathCameraComponent()->SetupAttachment(DeathCameraArm);
	GetDeathCameraComponent()->bUsePawnControlRotation = false;
	GetDeathCameraComponent()->FieldOfView = 110.0f;
	GetDeathCameraComponent()->bAutoActivate = false;

	// Climb detector
	ClimbSurfaceDetector = CreateDefaultSubobject<USphereComponent>(TEXT("ClimbSurfaceDetector"));
	GetClimbSurfaceDetector()->SetupAttachment(RootComponent);
	GetClimbSurfaceDetector()->InitSphereRadius(125.0f);
	GetClimbSurfaceDetector()->bGenerateOverlapEvents = true;
	GetClimbSurfaceDetector()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	GetClimbSurfaceDetector()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
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
	GetCharacterMovement()->CrouchedHalfHeight = 70.0f;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->AirControl = 0.7f;

	// Capsule overlap
	OnActorBeginOverlap.AddDynamic(this, &AMainCharacter::BeginOverlap);

	// Background music
	static ConstructorHelpers::FObjectFinder<USoundWave> BackgroundMusicAudioWaveObjectFinder(*FString("Blueprint'/Game/Music/PuzzleGame.PuzzleGame'"));
	BackgroundMusicAudioWave = BackgroundMusicAudioWaveObjectFinder.Object;

	// Blink sound
	static ConstructorHelpers::FObjectFinder<USoundWave> BlinkAudioWaveObjectFinder(*FString("Blueprint'/Game/SFX/Blink.Blink'"));
	BlinkAudioWave = BlinkAudioWaveObjectFinder.Object;

	// Bullet time start sound
	static ConstructorHelpers::FObjectFinder<USoundWave> BulletTimeStartAudioWaveObjectFinder(*FString("Blueprint'/Game/SFX/BulletTimeStart.BulletTimeStart'"));
	BulletTimeStartAudioWave = BulletTimeStartAudioWaveObjectFinder.Object;

	// Bullet time end sound
	static ConstructorHelpers::FObjectFinder<USoundWave> BulletTimeEndAudioWaveObjectFinder(*FString("Blueprint'/Game/SFX/BulletTimeEnd.BulletTimeEnd'"));
	BulletTimeEndAudioWave = BulletTimeEndAudioWaveObjectFinder.Object;

	// Death sound
	static ConstructorHelpers::FObjectFinder<USoundWave> DeathAudioWaveObjectFinder(*FString("Blueprint'/Game/SFX/Death/Death1.Death1'"));
	DeathAudioWave = DeathAudioWaveObjectFinder.Object;
	//int RandomNumber = rand() % 4 + 1; // Random number between 1 and 4
	//static ConstructorHelpers::FObjectFinder<USoundWave> RandomDeathAudioWaveObjectFinder(*FString("Blueprint'/Game/SFX/Death/Death" + FString::FromInt(RandomNumber) + ".Death" + FString::FromInt(RandomNumber) + "'"));
	//RandomDeathAudioWave = RandomDeathAudioWaveObjectFinder.Object;

	// Various
	static ConstructorHelpers::FClassFinder<UCameraShake> BlinkCameraShakeClassFinder(TEXT("Class'/Game/Blueprints/CameraShakes/BlinkCameraShake.BlinkCameraShake_C'"));
	BlinkCameraShake = BlinkCameraShakeClassFinder.Class;
	static ConstructorHelpers::FClassFinder<UUserWidget> HUDWidgetClassFinder(TEXT("Class'/Game/Blueprints/HUDs/UserHUD/UserHUDWidget.UserHUDWidget_C'"));
	HUDWidget = HUDWidgetClassFinder.Class;
	static ConstructorHelpers::FClassFinder<UUserWidget> PauseWidgetClassFinder(TEXT("Class'/Game/Blueprints/HUDs/MainMenu/MainMenuWidget.MainMenuWidget_C'"));
	PauseWidget = PauseWidgetClassFinder.Class;
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
		if (bIsWallRunning && !bIsClimbingLedge)
		{
			WallRunningStopDoOnce();
		}
	}

	// Debug utils
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("Actor location: %f - %f - %f"), GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("Actor rotation: %f - %f - %f"), GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("IsWallRunning: %s"), bIsWallRunning ? "T" : "F"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("IsHanging: %s"), bIsHanging ? "T" : "F"));
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("IsClimbing: %s"), bIsClimbingLedge ? "T" : "F"));
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());

	// Camera Setups
	PlayerController->PlayerCameraManager->ViewPitchMin = DefaultViewPitchMin;
	PlayerController->PlayerCameraManager->ViewPitchMax = DefaultViewPitchMax;

	// HUD initialization
	if (HUDWidget && PlayerController)
	{
		HUDWidgetHolder= UWidgetBlueprintLibrary::Create(GetWorld(), HUDWidget, PlayerController);
		HUDWidgetHolder->AddToViewport();
	}

	if (PauseWidget && PlayerController)
	{
		PauseWidgetHolder = UWidgetBlueprintLibrary::Create(GetWorld(), PauseWidget, PlayerController);
		PauseWidgetHolder->AddToViewport();
		PauseWidgetHolder->SetVisibility(ESlateVisibility::Hidden);
	}

	// Animation access util
	Animation = Cast<UHeroAnimationInstance>(GetMesh()->GetAnimInstance());

	// Start background music
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), BackgroundMusicAudioWave, GetActorLocation());

	// Save load
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Save"), 0))
	{
		UMainSaveGame* LoadGameInstance = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		LoadGameInstance = Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("Save"), 0));
		bool RespawnFromStart = LoadGameInstance->bFromStart;
		FName RespawnLevel = LoadGameInstance->Level;
		FName CurrentLevel = *UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("SameLevel: %s"), RespawnLevel == CurrentLevel ? "T" : "F"));

		if (!RespawnFromStart && (RespawnLevel == CurrentLevel))
		{
			bIsPowered = LoadGameInstance->bIsPowered;
			FVector RespawnLocation = LoadGameInstance->Location;
			FRotator RespawnRotation = LoadGameInstance->Rotation;
			this->SetActorLocation(RespawnLocation);
			Controller->SetControlRotation(RespawnRotation);
		}

		if (RespawnFromStart)
		{
			bIsPowered = false;
		}
	}
}

void AMainCharacter::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "Impact detected");

	// Dead management
	if (OtherActor->ActorHasTag(FName("deadly")))
	{
		if (!bIsDead)
		{
			OnDeath();
		}
	}
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

	// Pause
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AMainCharacter::OnPause).bExecuteWhenPaused = true;

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

void AMainCharacter::RotationTrick(float Value)
{
	Animation->RotationInput = Value;
}

void AMainCharacter::OnAction()
{
	
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
		AMainCharacter::JumpOrWallRunning();
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
				// Check if there is space to place the character after climb the ledge
				if (CanClimbLedge())
				{
					AMainCharacter::ClimbLedge();
				}
			}
		}
	}
}

void AMainCharacter::OnCrouchPressed()
{
	if (!bIsHanging)
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
	if (bIsPowered && !bIsHanging && !bIsClimbingLedge)
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
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), BulletTimeStartAudioWave, GetActorLocation(), 1.0f, 1.0f, 0.5f);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationWorldRate);
	this->CustomTimeDilation = TimeDilationPlayerRate;
	bTimeDilationIsActive = true;
}

void AMainCharacter::EndTimeDilation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), BulletTimeEndAudioWave, GetActorLocation());
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	this->CustomTimeDilation = 1.0f;
	bTimeDilationIsActive = false;
}

void AMainCharacter::OnBlink()
{
	if (bCanBlink && CurrentAdrenaline >= AdrenalinePerBlink) {
		bBlinkIsActive = true;
		ACharacter::Jump();

		// Timer handle
		FTimerHandle BlinkHandle;
		GetWorldTimerManager().SetTimer(BlinkHandle, this, &AMainCharacter::OnBlinkTimerEnd, 0.2f, false);
	}
}

void AMainCharacter::OnBlinkTimerEnd()
{
	PlayerController->ClientPlayCameraShake(BlinkCameraShake, 1.5f);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), BlinkAudioWave, GetActorLocation());
	this->SetActorLocationAndRotation(BlinkLocation + FVector(0.0f, 0.0f, 100.0f), GetCapsuleComponent()->GetComponentRotation(), true, nullptr, ETeleportType::None);
	CurrentAdrenaline = FMath::Max(CurrentAdrenaline - AdrenalinePerBlink, 0.0f);
	bBlinkIsActive = false;
}

void AMainCharacter::CheckCanBlink()
{
	if (bIsPowered && !bBlinkIsActive && !bIsHanging && !bIsClimbingLedge)
	{
		// Trace a line from the camera location in the direction where the player was looking
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("BlinkTracer")), true, this);
		RV_TraceParams.bTraceComplex = true;
		RV_TraceParams.bTraceAsyncScene = true;
		RV_TraceParams.bReturnPhysicalMaterial = false;
		RV_TraceParams.AddIgnoredActor(this);

		FHitResult RV_Hit_Blink(ForceInit);

		if (!bThirdPersonIsActive) {
			GetWorld()->LineTraceSingleByChannel(
				RV_Hit_Blink, // Result
				GetFirstPersonCameraComponent()->GetComponentLocation(), // Start
				GetFirstPersonCameraComponent()->GetComponentLocation() +
				(GetFirstPersonCameraComponent()->GetForwardVector() * BlinkMaximumDistance), // End
				ECollisionChannel::ECC_Visibility, // Collision channel
				RV_TraceParams
			);
		}
		else
		{
			GetWorld()->LineTraceSingleByChannel(
				RV_Hit_Blink, // Result
				GetThirdPersonCameraComponent()->GetComponentLocation(), // Start
				GetThirdPersonCameraComponent()->GetComponentLocation() +
				(GetThirdPersonCameraComponent()->GetForwardVector() * BlinkMaximumDistance), // End
				ECollisionChannel::ECC_Visibility, // Collision channel
				RV_TraceParams
			);
		}
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("BlinkTracer"));

		// If was a blocking hit, it return true and update the blink location too
		if (RV_Hit_Blink.bBlockingHit)
		{
			BlinkLocation = RV_Hit_Blink.ImpactPoint;
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

void AMainCharacter::OnPause()
{
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		// Check if the player isn't on a tutorial
		if (TutorialText == "")
		{
			UGameplayStatics::SetGamePaused(GetWorld(), false);

			PauseWidgetHolder->SetVisibility(ESlateVisibility::Hidden);
			HUDWidgetHolder->SetVisibility(ESlateVisibility::Visible);

			UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
			PlayerController->bShowMouseCursor = false;
		}
	}
	else
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		HUDWidgetHolder->SetVisibility(ESlateVisibility::Hidden);
		PauseWidgetHolder->SetVisibility(ESlateVisibility::Visible);
		
		UWidgetBlueprintLibrary::SetInputMode_GameAndUI(PlayerController, PauseWidgetHolder, true); // Change bool to "EMouseLockMode::LockAlways"
		PlayerController->bShowMouseCursor = true;
	}
}

void AMainCharacter::OnCameraToggle()
{
	if (!bIsDead)
	{
		GetFirstPersonCameraComponent()->ToggleActive();
		GetThirdPersonCameraComponent()->ToggleActive();
	}

	if (GetFirstPersonCameraComponent()->IsActive()) {
		bThirdPersonIsActive = false;
	}
	else
	{
		bThirdPersonIsActive = true;
	}
}

void AMainCharacter::OnClimbSurfaceDetectorBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	TArray<AActor*> OverlappingActorsArray;
	GetClimbSurfaceDetector()->GetOverlappingActors(OverlappingActorsArray);
	bCanTrace = OverlappingActorsArray.Num() > 0;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "Sphere begin overlap");
}

void AMainCharacter::OnClimbSurfaceDetectorEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TArray<AActor*> OverlappingActorsArray;
	GetClimbSurfaceDetector()->GetOverlappingActors(OverlappingActorsArray);
	bCanTrace = OverlappingActorsArray.Num() > 0;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "Sphere end overlap");
}

void AMainCharacter::DoTrace()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "Climbing system tracing");
	if ((GetCharacterMovement()->IsFalling() && !bIsHanging) || bIsWallRunning)
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
				ECollisionChannel::ECC_Visibility, // Collision channel
				FCollisionShape::MakeSphere(20.0f), // Radius
				RV_TraceParams_Roof
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("RoofTracer"));

			if (RV_Hit_Roof.bBlockingHit)
			{
				if (bIsWallRunning)
				{
					WallRunningStopDoOnce();
				}
				else
				{
					WallRunningStopReset();
				}
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
			GetActorLocation() + FVector(0, 0, 500.0f) + (GetActorForwardVector() * 55.0f) - (GetActorRightVector() * 35.0f), // Start
			GetActorLocation() + FVector(0, 0, 500.0f) + (GetActorForwardVector() * 55.0f) - (GetActorRightVector() * 35.0f) - FVector(0, 0, 550.0f), // End
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
			GetActorLocation() + FVector(0, 0, 500.0f) + (GetActorForwardVector() * 55.0f) + (GetActorRightVector() * 35.0f), // Start
			GetActorLocation() + FVector(0, 0, 500.0f) + (GetActorForwardVector() * 55.0f) + (GetActorRightVector() * 35.0f) - FVector(0, 0, 550.0f), // End
			FQuat(),
			ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
			FCollisionShape::MakeSphere(10.0f), // Radius
			RV_TraceParams_Height_Right
		);
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("HeightTracerRight"));

		// Floor tracer
		FCollisionQueryParams RV_TraceParams_Floor = FCollisionQueryParams(FName(TEXT("FloorTracer")), true, this);
		RV_TraceParams_Floor.bTraceComplex = true;
		RV_TraceParams_Floor.bTraceAsyncScene = true;
		RV_TraceParams_Floor.bReturnPhysicalMaterial = false;

		FHitResult RV_Hit_Floor(ForceInit);

		GetWorld()->SweepSingleByChannel(
			RV_Hit_Floor, // Result
			GetActorLocation(), // Start
			GetActorLocation() + FVector(0, 0, -250.0f), // End
			FQuat(),
			ECollisionChannel::ECC_Visibility, // Collision channel
			FCollisionShape::MakeSphere(20.0f), // Radius
			RV_TraceParams_Floor
		);
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("FloorTracer"));

		if (RV_Hit_Height_Left.bBlockingHit && RV_Hit_Height_Right.bBlockingHit)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, "Height tracer succeed");
			LedgeHeight = RV_Hit_Height_Left.ImpactPoint; // We take the left impact point, at the moment

			// Climbing checks
			if (!bIsWallRunning)
			{
				if ((((GetMesh()->GetSocketLocation(TEXT("HipSocket"))).Z - (LedgeHeight.Z)) >= -70.0f)
					&& (((GetMesh()->GetSocketLocation(TEXT("HipSocket"))).Z - (LedgeHeight.Z)) <= 0.0f)
					&& !RV_Hit_Floor.bBlockingHit)
				{
					GrabLedgeDoOnce();
				}
				else
				{
					GrabLedgeReset();
					KneeClimbLedge();
				}
			}
			else
			{
				GrabLedgeReset();
				KneeClimbLedge();
			}
		}
	}

	// Check if the player will be doing braced hanging or doing free hanging
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
			GetActorLocation() + FVector(0, 0, -65.0f) - (GetActorRightVector() * 35.0f), // Start
			GetActorLocation() + FVector(0, 0, -65.0f) + (GetActorForwardVector() * FVector(50.0f, 50.0f, 0)) - (GetActorRightVector() * 35.0f), // End
			FQuat(),
			ECollisionChannel::ECC_Visibility, // Collision channel
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
			GetActorLocation() + FVector(0, 0, -65.0f) + (GetActorRightVector() * 35.0f), // Start
			GetActorLocation() + FVector(0, 0, -65.0f) + (GetActorForwardVector() * FVector(50.0f, 50.0f, 0)) + (GetActorRightVector() * 35.0f), // End
			FQuat(),
			ECollisionChannel::ECC_Visibility, // Collision channel
			FCollisionShape::MakeSphere(10.0f), // Radius
			RV_TraceParams_Foot_Right
		);
		//GetWorld()->DebugDrawTraceTag = FName(TEXT("FootTracerRight"));

		// If the two traces hit, the character can keep their feet close to the wall (braced hanging)
		if (RV_Hit_Foot_Left.bBlockingHit || RV_Hit_Foot_Right.bBlockingHit)
		{
			bCanBraceHang = true;
			Animation->bCanBraceHang = true;
			if (!bIsGrabbingLookingRear)
			{
				RestrictView(AlignToWall().Yaw - 60.0f, AlignToWall().Yaw + 60.0f, -60.0f);
			}
		}
		else
		{
			bCanBraceHang = false;
			Animation->bCanBraceHang = false;
			RestrictView(AlignToWall().Yaw - 50.0f, AlignToWall().Yaw + 50.0f, -35.0f);
		}
	}
}

void AMainCharacter::GrabLedgeDoOnce()
{
	if (bGrabLedgeDoOnce)
	{
		bGrabLedgeDoOnce = false;

		if (!bIsClimbingLedge)
		{
			GetFirstPersonCameraArm()->ProbeSize = HangingFPCameraProbeSize;
			GetMesh()->RelativeLocation = FVector(0.0f, 0.0f, -CapsuleHalfHeight);

			Animation->bIsHanging = true;

			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			bIsHanging = true;

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
				0.1f, // Over Time
				true, // Force Rotation Shortest Path
				EMoveComponentAction::Move, // Movement Type
				GrabLedgeMoveLatentInfo);

			GetCharacterMovement()->StopMovementImmediately();
			bCanDoWallRunning = true;
			
			// Camera resctrictions
			bUseControllerRotationYaw = false;
			RestrictView(AlignToWall().Yaw - 60.0f, AlignToWall().Yaw + 60.0f, -60.0f);
		}
	}
}

void AMainCharacter::GrabLedgeReset()
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
	
	// Camera restrictions
	bUseControllerRotationYaw = true;
	ResetRestrictView();
	GetFirstPersonCameraArm()->ProbeSize = DefaultFPCameraProbeSize;
	GetMesh()->RelativeLocation = FVector(-7.5f, 0.0f, -CapsuleHalfHeight);
}

void AMainCharacter::ClimbLedge()
{
	if (!bIsClimbingLedge)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

		bIsClimbingLedge = true;
		bIsHanging = false;

		Animation->Climbing();
		Animation->bIsHanging = false;

		// Camera restrictions
		bUseControllerRotationYaw = false;
		RestrictView(AlignToWall().Yaw, AlignToWall().Yaw);
	}
}

void AMainCharacter::KneeClimbLedge()
{
	//float HipToLedgeHeightDistance;
	//HipToLedgeHeightDistance = GetMesh()->GetSocketLocation(TEXT("HipSocket")).Z - LedgeHeight.Z;

	if ((AMainCharacter::GetVelocity().Z > 0.0f && ((GetMesh()->GetSocketLocation(TEXT("HipSocket")).Z - LedgeHeight.Z) >= 0.0f && (GetMesh()->GetSocketLocation(TEXT("HipSocket")).Z - LedgeHeight.Z) <= 20.0f)) 
		|| (AMainCharacter::GetVelocity().Z <= 0.0f && (GetMesh()->GetSocketLocation(TEXT("HipSocket")).Z - LedgeHeight.Z) == 1.0f))
	{
		if (!bIsWallRunning || (bIsWallRunning && (AMainCharacter::GetVelocity().Z < 300.0f)))
		{
			if (CanClimbLedge())
			{
				KneeClimbLedgeDoOnce();
			}
		}
		else
		{
			KneeClimbLedgeDoOnceReset();
		}
	}
	else
	{
		KneeClimbLedgeDoOnceReset();
	}
}

void AMainCharacter::KneeClimbLedgeDoOnce()
{
	if (bKneeClimbLedgeDoOnce)
	{
		bKneeClimbLedgeDoOnce = false;

		if (!bIsClimbingLedge)
		{
			Animation->ClimbingKnees();
			bIsClimbingLedge = true;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			bUseControllerRotationYaw = false;

			FLatentActionInfo KneeClimbingLatentInfo;
			KneeClimbingLatentInfo.CallbackTarget = this;
			UKismetSystemLibrary::MoveComponentTo(RootComponent,
				FVector(
					(WallNormal * FVector(CapsuleRadius, CapsuleRadius, CapsuleRadius)).X + WallImpact.X,
					(WallNormal * FVector(CapsuleRadius, CapsuleRadius, CapsuleRadius)).Y + WallImpact.Y,
					LedgeHeight.Z + 70.0f),
				AlignToWall(),
				false, // Ease Out
				false, // Ease In
				0.2f, // Over Time
				false, // Force Rotation Shortest Path
				EMoveComponentAction::Move, // Movement Type
				KneeClimbingLatentInfo);

			GetCharacterMovement()->StopMovementImmediately();
			bIsWallRunning = false;

			// Camera restrictions
			RestrictView(AlignToWall().Yaw, AlignToWall().Yaw, -60.0f);
		}
	}
}

void AMainCharacter::KneeClimbLedgeDoOnceReset()
{
	bKneeClimbLedgeDoOnce = true;
}

void AMainCharacter::CompleteClimb()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	bIsClimbingLedge = false;
	bIsHanging = false;
	bIsWallRunning = false;

	// Camera restrictions
	bUseControllerRotationYaw = true;
	ResetRestrictView();
	GetFirstPersonCameraArm()->ProbeSize = DefaultFPCameraProbeSize;
	GetMesh()->RelativeLocation = FVector(-7.5f, 0.0f, -CapsuleHalfHeight);
}

bool AMainCharacter::CanClimbLedge()
{
	// Can climb tracer
	FCollisionQueryParams RV_TraceParams_Can_Climb = FCollisionQueryParams(FName(TEXT("CanClimbTracer")), true, this);
	RV_TraceParams_Can_Climb.bTraceComplex = true;
	RV_TraceParams_Can_Climb.bTraceAsyncScene = true;
	RV_TraceParams_Can_Climb.bReturnPhysicalMaterial = false;

	FHitResult RV_Hit_Can_Climb(ForceInit);

	GetWorld()->SweepSingleByChannel(
		RV_Hit_Can_Climb, // Result
		GetActorLocation() + (GetActorForwardVector() * 80.0f) + FVector(0, 0, 160.0f), // Start
		GetActorLocation() + (GetActorForwardVector() * 80.0f) + FVector(0, 0, 160.0f + (CapsuleHalfHeight * 2)), // End
		FQuat(),
		ECollisionChannel::ECC_Visibility, // Collision channel
		FCollisionShape::MakeSphere(CapsuleRadius), // Radius
		RV_TraceParams_Can_Climb
	);
	//GetWorld()->DebugDrawTraceTag = FName(TEXT("CanClimbTracer"));

	if (RV_Hit_Can_Climb.bBlockingHit)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AMainCharacter::GrabLedgeMove()
{
	if (GetInputAxisValue(TEXT("MoveRight")) == 0.0f || bIsGrabbingLookingRear)
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
					GetActorLocation() + FVector(0, 0, 125.0f) + (GetActorForwardVector() * 50.0f) + (GetActorRightVector() * FVector(50.0f, 50.0f, 0.0f)), // Start
					GetActorLocation() + FVector(0, 0, 125.0f) + (GetActorForwardVector() * 50.0f) + (GetActorRightVector() * 350.0f), // End
					FQuat(),
					ECollisionChannel::ECC_Visibility, // Collision channel
					FCollisionShape::MakeSphere(2.5f), // Radius
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
					GetActorLocation() + FVector(0, 0, 125.0f) + (GetActorForwardVector() * 50.0f) - (GetActorRightVector() * FVector(50.0f, 50.0f, 0.0f)), // Start
					GetActorLocation() + FVector(0, 0, 125.0f) + (GetActorForwardVector() * 50.0f) - (GetActorRightVector() * 350.0f), // End
					FQuat(),
					ECollisionChannel::ECC_Visibility, // Collision channel
					FCollisionShape::MakeSphere(2.5f), // Radius
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
		GetCharacterMovement()->StopMovementImmediately();

		// Camera restrictions
		bUseControllerRotationYaw = false;
		RestrictView(AlignToWall().Yaw - 175.0f, AlignToWall().Yaw - 60.0f, -60.0f);
	}
}

void AMainCharacter::GrabLedgeRearCancel()
{
	if (bIsGrabbingLookingRear)
	{
		bIsGrabbingLookingRear = false;
		Animation->bIsGrabingAndLookingRear = false;
		GetCharacterMovement()->StopMovementImmediately();

		// Camera restrictions
		bUseControllerRotationYaw = false;
		RestrictView(AlignToWall().Yaw - 60.0f, AlignToWall().Yaw + 60.0f, -60.0f);
	}
}

void AMainCharacter::JumpGrabbingSide()
{
	LeaveLedge();
	if (GetInputAxisValue(TEXT("MoveRight")) > 0.0f)
	{
		LaunchCharacter((GetActorRightVector() * 500.0f) + FVector(0, 0, 650.0f), false, false);
	}
	else
	{
		if (GetInputAxisValue(TEXT("MoveRight")) < 0.0f)
		{
			LaunchCharacter((UKismetMathLibrary::NegateVector(GetActorRightVector()) * 500.0f) + FVector(0, 0, 650.0f), false, false);
		}
	}
}

void AMainCharacter::JumpGrabbingRear()
{
	LeaveLedge();
	LaunchCharacter((WallNormal * 500.0f) + FVector(0, 0, 300.0f), false, false);
	PlayerController->SetControlRotation(GetActorRotation() + FRotator(0, 180.0f, 0));
}

void AMainCharacter::JumpOrWallRunning()
{
	if (bIsWallRunning)
	{
		WallRunningRearJump();
	}
	else
	{
		if (!GetCharacterMovement()->IsFlying())
		{
			if (GetCharacterMovement()->IsWalking())
			{
				bCanDoWallRunning = true;
			}

			// TODO: Check if the player is busy

			// Wall run tracer
			FCollisionQueryParams RV_TraceParams_Wall_Run = FCollisionQueryParams(FName(TEXT("WallRunTracer")), true, this);
			RV_TraceParams_Wall_Run.bTraceComplex = true;
			RV_TraceParams_Wall_Run.bTraceAsyncScene = true;
			RV_TraceParams_Wall_Run.bReturnPhysicalMaterial = false;

			FHitResult RV_Hit_Wall_Run(ForceInit);

			GetWorld()->SweepSingleByChannel(
				RV_Hit_Wall_Run, // Result
				GetActorLocation(), // Start
				GetActorLocation() + (GetActorForwardVector() * FVector(150.0f, 150.0f, 0)), // End
				FQuat(),
				ECollisionChannel::ECC_GameTraceChannel2, // Collision channel
				FCollisionShape::MakeSphere(20.0f), // Radius
				RV_TraceParams_Wall_Run
			);
			//GetWorld()->DebugDrawTraceTag = FName(TEXT("WallRunTracer"));

			if (RV_Hit_Wall_Run.bBlockingHit && bCanDoWallRunning)
			{
				WallImpact = RV_Hit_Wall_Run.ImpactPoint;
				WallNormal = RV_Hit_Wall_Run.ImpactNormal;

				Animation->bIsWallRunnnig = true;
				Animation->WallRunning();

				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
				bUseControllerRotationYaw = false;

				FLatentActionInfo WallRunningLatentInfo;
				WallRunningLatentInfo.CallbackTarget = this;
				UKismetSystemLibrary::MoveComponentTo(RootComponent,
					FVector(
						WallImpact.X - (GetActorForwardVector().X * 40.0f),
						WallImpact.Y - (GetActorForwardVector().Y * 40.0f),
						WallImpact.Z + 100.0f),
					AlignToWall(),
					false, // Ease Out
					false, // Ease In
					0.2f, // Over Time
					true, // Force Rotation Shortest Path
					EMoveComponentAction::Move, // Movement Type
					WallRunningLatentInfo);

				bIsWallRunning = true;
				bCanDoWallRunning = false;

				// Camera restrictions
				RestrictView(AlignToWall().Yaw - 60.0f, AlignToWall().Yaw + 60.0f, -60.0f);
			}
			else
			{
				ACharacter::Jump();
			}
		}
	}
}

void AMainCharacter::WallRunningRearJump()
{
	if (bIsWallRunning && !bIsClimbingLedge)
	{
		WallRunningStop();
		PlayerController->SetControlRotation(GetActorRotation() + FRotator(0, 180.0f, 0));
		LaunchCharacter((WallNormal * 250.0f) + FVector(0, 0, 300.0f), false, false);
		bCanDoWallRunning = true;
	}
}

void AMainCharacter::WallRunningStop()
{
	Animation->bIsWallRunnnig = false;
	Animation->WallRunningStop();

	GetCharacterMovement()->StopMovementImmediately();

	bIsWallRunning = false;
	bIsClimbingLedge = false;

	if (!bIsHanging)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

		// Camera restrictions
		bUseControllerRotationYaw = true;
		ResetRestrictView();
	}
}

void AMainCharacter::WallRunningStopDoOnce()
{
	if (bWallRunningStopDoOnce)
	{
		bWallRunningStopDoOnce = false;
		WallRunningStop();
	}
}

void AMainCharacter::WallRunningStopReset()
{
	bWallRunningStopDoOnce = true;
}

void AMainCharacter::OnDeath()
{
	if (!bIsDead)
	{
		bIsDead = true;
		if (bTimeDilationIsActive)
		{
			EndTimeDilation();
		}
		GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
		this->DisableInput(PlayerController);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		GetCharacterMovement()->StopMovementImmediately();

		// Camera management
		GetFirstPersonCameraComponent()->SetActive(false);
		GetThirdPersonCameraComponent()->SetActive(false);
		GetDeathCameraComponent()->SetActive(true);
		PlayerController->ClientSetCameraFade(true, FColor::Red, FVector2D(0.75, 0.0), 2.5f);

		// Sound
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathAudioWave, GetActorLocation());

		// Timer Handle
		FTimerHandle DeadHandle;
		GetWorldTimerManager().SetTimer(DeadHandle, this, &AMainCharacter::OnDeathEnd, 3.0f, false);
	}
}

void AMainCharacter::OnDeathEnd()
{
	FName RespawnLevel = FName("Tutorial"); // Default (first) map
	
	// Save load
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Save"), 0))
	{
		UMainSaveGame* LoadGameInstance = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		LoadGameInstance = Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("Save"), 0));
		RespawnLevel = LoadGameInstance->Level;
		UGameplayStatics::OpenLevel(GetWorld(), RespawnLevel);
	}

	UGameplayStatics::OpenLevel(GetWorld(), RespawnLevel);
	PlayerController->ClientSetCameraFade(true, FColor::Black, FVector2D(1.0, 0.0), 100.0f);
	GetMesh()->SetSimulatePhysics(false);
	bIsDead = false;
	GetFirstPersonCameraComponent()->SetActive(true);
	GetThirdPersonCameraComponent()->SetActive(false);
	GetDeathCameraComponent()->SetActive(false);
	this->EnableInput(PlayerController);
	GetMesh()->SetCollisionProfileName(FName("CharacterMesh"));
}

//////////////////////////////////////////////////////////////////////////
// Util

FRotator AMainCharacter::AlignToWall()
{
	return UKismetMathLibrary::MakeRotFromXZ((WallNormal * FVector(-1, -1, 0)), GetCapsuleComponent()->GetUpVector());
}

void AMainCharacter::ResetRestrictView()
{
	PlayerController->PlayerCameraManager->ViewYawMin = DefaultViewYawMin;
	PlayerController->PlayerCameraManager->ViewYawMax = DefaultViewYawMax;
	PlayerController->PlayerCameraManager->ViewPitchMin = DefaultViewPitchMin;
}

void AMainCharacter::RestrictView(float YawMin, float YawMax)
{
	float Aux = DefaultViewPitchMin;

	RestrictView(YawMin, YawMax, Aux);
}

void AMainCharacter::RestrictView(float YawMin, float YawMax, float PitchMin)
{
	float YawNewCenter;
	YawNewCenter = (YawMin + YawMax) / 2;

	//UKismetMathLibrary::RInterpTo(
	//	GetActorRotation(), // Current rotation
	//	FRotator(GetActorRotation().Pitch, YawNewCenter, GetActorRotation().Roll), // New rotation
	//	4.0f, // Time
	//	1.0f //Velocity
	//);
	
	PlayerController->PlayerCameraManager->ViewYawMin = YawMin;
	PlayerController->PlayerCameraManager->ViewYawMax = YawMax;
	PlayerController->PlayerCameraManager->ViewPitchMin = PitchMin;
}

//////////////////////////////////////////////////////////////////////////
// Interface

void AMainCharacter::HUDInterface_Implementation(bool &bCanBlink_Interface, float &CurrentAdrenaline_Interface, float &MaximumAdrenaline_Interface, bool &bIsDead_Interface, FString &TutorialText_Interface)
{
	bCanBlink_Interface = bCanBlink;
	CurrentAdrenaline_Interface = CurrentAdrenaline;
	MaximumAdrenaline_Interface = MaximumAdrenaline;
	bIsDead_Interface = bIsDead;
	TutorialText_Interface = TutorialText;
}
