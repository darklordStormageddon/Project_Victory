// Copyright Epic Games, Inc. All Rights Reserved.

#include "TeamSpaceProjectGameMode.h"
#include "TeamSpaceProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATeamSpaceProjectGameMode::ATeamSpaceProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
