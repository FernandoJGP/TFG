// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#pragma once

#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UCLASS()
class TFG_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

	// First person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
	
	// First person camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* FirstPersonCameraArm;

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

// Components gets
public:
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetFirstPersonCameraArm() const { return FirstPersonCameraArm; }
};
