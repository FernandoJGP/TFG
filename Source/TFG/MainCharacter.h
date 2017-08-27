// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#pragma once

#include "GameFramework/Character.h"
#include "HUDInterface.h"

#include "MainCharacter.generated.h"

UCLASS()
class TFG_API AMainCharacter : public ACharacter, public IHUDInterface
{
	GENERATED_BODY()

	// First person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
	
	// First person camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* FirstPersonCameraArm;

	// Third person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ThirdPersonCameraComponent;

	// Third person forward-backward camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* ThirdPersonForwardBackwardCameraArm;

	// Third person right-left camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* ThirdPersonLeftRightCameraArm;

	// Dead camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* DeathCameraComponent;

	// Dead camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* DeathCameraArm;

	// Climb surface detector
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ClimbDetector", meta = (AllowPrivateAccess = "true"))
	USphereComponent* ClimbSurfaceDetector;

public:
	// Sets default values for this character's properties
	AMainCharacter();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Properties visibles from blueprint
public:
	// Base turn rate, in deg/sec
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;

	// Base look up/down rate, in deg/sec
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;

	// Stores if the third person camera is active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bThirdPersonIsActive = false;

	// Stores if the player is dead or not
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dead")
	bool bIsDead = false;

	// Stores if the player has powers currently
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Power")
	bool bIsPowered = false;

	// Maximum adrenaline that the player can store
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adrenaline Management")
	float MaximumAdrenaline = 1000.0f;

	// Current adrenaline that the player have
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adrenaline Management")
	float CurrentAdrenaline = MaximumAdrenaline;

	// Adrenaline recovered per tick
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adrenaline Management")
	float AdrenalinePerTick = 5.0f;

	// Checks if time dilation it's active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time Dilation")
	bool bTimeDilationIsActive;

	// Bullet time world rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time Dilation")
	float TimeDilationWorldRate = 0.5f;

	// Bullet time player rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time Dilation")
	float TimeDilationPlayerRate = 0.75f;

	// Adrenaline that the player loss per tick when use bullet time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time Dilation")
	float AdrenalinePerTimeDilation = 2.5f;

	// Stores if the player can blink
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blink")
	bool bCanBlink;

	// Stores if the player is teleporting himself
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blink")
	bool bBlinkIsActive;

	// Maximum blink distance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blink")
	float BlinkMaximumDistance = 2048.0f;

	// Stores the blink impact point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blink")
	FVector BlinkLocation;

	// Adrenaline that the player loss when use blink
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blink")
	float AdrenalinePerBlink = 250.0f;

	// Stores if the player can trace
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bCanTrace = false;

	// Stores if the player is hanging
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bIsHanging = false;

	// Stores if the player can do braced hanging
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bCanBraceHang = false;

	// Stores if the player is hanging looking to a side
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bIsGrabbingLookingSide = false;

	// Stores if the player is hanging looking rear
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bIsGrabbingLookingRear = false;

	// Stores if the player is climbing
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Climb")
	bool bIsClimbingLedge = false;

	// Stores if the player is doing wall running
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Climb")
	bool bIsWallRunning = false;

	// Stores if the player can do wall running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bCanDoWallRunning = true;

	// Stores the ledge height point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	FVector LedgeHeight;

	// Stores the wall impact point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	FVector WallImpact;

	// Stores the wall impact normal
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	FVector WallNormal;

	// Grab ledge do once aux variable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bGrabLedgeDoOnce = true;

	// Knee climb do once aux variable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bKneeClimbLedgeDoOnce = true;

	// Wall running stop do once aux variable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
	bool bWallRunningStopDoOnce = true;

	// Tutorial text
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FString TutorialText = "";

// Various
	// HUD widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<class UUserWidget> HUDWidget;
	UUserWidget* HUDWidgetHolder;

	// Pause widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<class UUserWidget> PauseWidget;
	UUserWidget* PauseWidgetHolder;

	// Blink camera shake
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraShake")
	TSubclassOf<class UCameraShake> BlinkCameraShake;

	// Background music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundWave* BackgroundMusicAudioWave;

	// Blink sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundWave* BlinkAudioWave;

	// Bullet time start sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundWave* BulletTimeStartAudioWave;

	// Bullet end sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundWave* BulletTimeEndAudioWave;

	// Death sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundWave* DeathAudioWave;

// Functions
private:
	// Overlap
	UFUNCTION()
	void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	// Handles moving forward/backward
	void MoveForward(float Val);

	// Handles stafing movement, left and right
	void MoveRight(float Val);

	// Horizontal camera movement for pad controllers
	void TurnAtRate(float Rate);

	// Vertical camera movement for pad controllers
	void LookUpAtRate(float Rate);

	// Tick to know when the player is on rotation
	void RotationTrick(float Rate);

	// Action button
	void OnAction();

	// Sprint
	void OnSprintPressed();
	void OnSprintReleased();

	// Jump
	void OnJump();

	// Crouch
	void OnCrouchPressed();
	void OnCrouchReleased();

	// Time dilation
	void OnTimeDilationToggle();
	void StartTimeDilation();
	void EndTimeDilation();

	// Blink
	void OnBlink();
	void CheckCanBlink();
	void OnBlinkTimerEnd();

	// Pause
	UFUNCTION(BlueprintCallable)
	void OnPause();

	// Camera toggle
	void OnCameraToggle();

	UFUNCTION(BlueprintCallable)
	// Death
	void OnDeath();

	// Death end
	void OnDeathEnd();

	// Climb surface detector
	UFUNCTION()
	void OnClimbSurfaceDetectorBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnClimbSurfaceDetectorEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void DoTrace();
	void GrabLedgeDoOnce();
	void GrabLedgeReset();
	void LeaveLedge();
	void ClimbLedge();
	void KneeClimbLedge();
	void KneeClimbLedgeDoOnce();
	void KneeClimbLedgeDoOnceReset();
	bool CanClimbLedge();
	UFUNCTION(BlueprintCallable)
	void CompleteClimb();
	void GrabLedgeMove();
	void GrabLedgeRear();
	void GrabLedgeRearCancel();
	void JumpGrabbingSide();
	void JumpGrabbingRear();
	void JumpOrWallRunning();
	void WallRunningRearJump();
	UFUNCTION(BlueprintCallable)
	void WallRunningStop();
	void WallRunningStopDoOnce();
	void WallRunningStopReset();

	FRotator AlignToWall();
	void RestrictView(float YawMin, float YawMax);
	void RestrictView(float YawMin, float YawMax, float PitchMin);
	void ResetRestrictView();

// Interface methods
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	void HUDInterface(bool &bCanBlink_Interface, float &CurrentAdrenaline_Interface, float &MaximumAdrenaline_Interface, bool &bIsDead_Interface, FString &TutorialText_Interface);
	virtual void HUDInterface_Implementation(bool &bCanBlink_Interface, float &CurrentAdrenaline_Interface, float &MaximumAdrenaline_Interface, bool &bIsDead_Interface, FString &TutorialText_Interface);

//Util methods
private:
	// Player controller util
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller", meta = (AllowPrivateAccess = "true"))
	class APlayerController* PlayerController;

	// Animation util
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	class UHeroAnimationInstance* Animation;

// Components gets
public:
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetFirstPersonCameraArm() const { return FirstPersonCameraArm; }
	FORCEINLINE class UCameraComponent* GetThirdPersonCameraComponent() const { return ThirdPersonCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetThirdPersonForwardBackwardCameraArm() const { return ThirdPersonForwardBackwardCameraArm; }
	FORCEINLINE class USpringArmComponent* GetThirdPersonLeftRightCameraArm() const { return ThirdPersonLeftRightCameraArm; }
	FORCEINLINE class UCameraComponent* GetDeathCameraComponent() const { return DeathCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetDeathCameraArm() const { return DeathCameraArm; }
	FORCEINLINE class USphereComponent* GetClimbSurfaceDetector() const { return ClimbSurfaceDetector; }
};
