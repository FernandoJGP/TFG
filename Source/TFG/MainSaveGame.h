// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#pragma once

#include "GameFramework/SaveGame.h"
#include "MainSaveGame.generated.h"

UCLASS()
class TFG_API UMainSaveGame : public USaveGame
{
	GENERATED_BODY()
	
	public:

	UPROPERTY(VisibleAnywhere)
	FName Level;

	UPROPERTY(VisibleAnywhere)
	bool bFromStart;

	UPROPERTY(VisibleAnywhere)
	FVector Location;
	
	UPROPERTY(VisibleAnywhere)
	FRotator Rotation;
};
