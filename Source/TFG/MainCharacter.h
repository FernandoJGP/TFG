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

	// Third person camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* ThirdPersonCameraArm;

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

	// Stores if the player has powers currently
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
		bool bIsPowered = true; // For debugging

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
		float TimeDilationWorldRate = 0.25f;

	// Bullet time player rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time Dilation")
		float TimeDilationPlayerRate = 0.5f;

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

	// User widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD")
		TSubclassOf<class UUserWidget> HUDWidget;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
		bool bIsClimbingLedge = false;

	// Stores the ledge height point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
		FVector LedgeHeight;

	// Stores the wall impact point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
		FVector WallImpact;

	// Stores the wall impact normal
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
		FVector WallNormal;

	// Stores the wall impact normal
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb")
		bool bGrabLedgeDoOnce = true;

// Functions
private:
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

	// Camera toggle
	void OnCameraToggle();

	// Climb surface detector
	UFUNCTION()
	void OnClimbSurfaceDetectorBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnClimbSurfaceDetectorEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void DoTrace();
	void GrabLedge();
	void ResetGrabLedge();
	void LeaveLedge();
	void KneeClimb();
	void GrabLedgeMove(float Value);
	void GrabLedgeRear();
	void GrabLedgeRearCancel();

	FRotator AlignToWall();

// Interface methods
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	void HUDInterface(bool &bCanBlink_Interface, float &CurrentAdrenaline_Interface, float &MaximumAdrenaline_Interface);
	virtual void HUDInterface_Implementation(bool &bCanBlink_Interface, float &CurrentAdrenaline_Interface, float &MaximumAdrenaline_Interface);

//Util methods
private:
	// Animation util
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	class UHeroAnimationInstance* Animation;

// Components gets
public:
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetFirstPersonCameraArm() const { return FirstPersonCameraArm; }
	FORCEINLINE class UCameraComponent* GetThirdPersonCameraComponent() const { return ThirdPersonCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetThirdPersonCameraArm() const { return ThirdPersonCameraArm; }
	FORCEINLINE class USphereComponent* GetClimbSurfaceDetector() const { return ClimbSurfaceDetector; }
};
