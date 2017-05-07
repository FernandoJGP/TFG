// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#pragma once

#include "Animation/AnimInstance.h"
#include "HeroAnimationInstance.generated.h"

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class TFG_API UHeroAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrouched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ClimbingSystem")
	bool bIsHanging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingSystem")
	bool bCanBraceHang;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingSystem")
	bool bIsClimbing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingSystem")
	bool bIsWallRunnnig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingSystem")
	float GrabRightInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingSystem")
	bool bIsGrabingAndLookingRear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingSystem")
	float GrabLookingRight;
};
