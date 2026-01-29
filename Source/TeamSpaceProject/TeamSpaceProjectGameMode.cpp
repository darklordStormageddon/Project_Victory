// Copyright Epic Games, Inc. All Rights Reserved.

#include "TeamSpaceProjectGameMode.h"
#include "TeamSpaceProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATeamSpaceProjectGameMode::ATeamSpaceProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Main/PS_PSJ/Blueprint/PlayerCharacter/MyPSJ_Character"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}