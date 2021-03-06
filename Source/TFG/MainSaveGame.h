// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#pragma once

#include "GameFramework/SaveGame.h"
#include "MainSaveGame.generated.h"

UCLASS()
class TFG_API UMainSaveGame : public USaveGame
{
	GENERATED_BODY()
	
	public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bFromStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsPowered;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector Location;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FRotator Rotation;
};
