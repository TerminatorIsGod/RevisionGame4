// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevisionGame4GameMode.h"
#include "RevisionGame4Character.h"
#include "UObject/ConstructorHelpers.h"

ARevisionGame4GameMode::ARevisionGame4GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
