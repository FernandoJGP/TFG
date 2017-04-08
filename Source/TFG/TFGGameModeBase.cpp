// Copyright (C) 2017 - Fernando Jos� Garc�a Padilla - Universidad de Sevilla

#include "TFG.h"
#include "TFGGameModeBase.h"

ATFGGameModeBase::ATFGGameModeBase()
	: Super()
{
	// Default pawn
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/MainCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}