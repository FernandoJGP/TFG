// Copyright (C) 2017 - Fernando José García Padilla - Universidad de Sevilla

#pragma once

#include "HUDInterface.generated.h"

UINTERFACE(BlueprintType)
class UHUDInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class TFG_API IHUDInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HUDInterface(bool &bCanBlink, float &CurrentAdrenaline, float &MaximumAdrenaline, bool &IsDead, FString &TutorialText);
};
