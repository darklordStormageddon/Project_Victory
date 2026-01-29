// Fill out your copyright notice in the Description page of Project Settings.

#include "PSJ_GameModeBase.h"
#include "PSJ_Character.h"
#include "PSJ_PlayerController.h"
#include "UObject/ConstructorHelpers.h"

APSJ_GameModeBase::APSJ_GameModeBase()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Main/PS_PSJ/Blueprint/PlayerCharacter/MyPSJ_Character"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

