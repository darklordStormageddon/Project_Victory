// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

AJHSGameMode::AJHSGameMode()
{
	_spaceObjectManager = CreateDefaultSubobject<USpaceObjectManager>(TEXT("SpaceObjectManager"));
}

void AJHSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AJHSGameMode::BeginPlay()
{
	Super::BeginPlay();
}

