// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/SpaceManager.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"

AJHSGameMode::AJHSGameMode()
{
	_uiManager = CreateDefaultSubobject<UUIManager>(TEXT("UIManager"));
	_eventManager = CreateDefaultSubobject<UEventManager>(TEXT("EventManager"));
	_spaceManager = CreateDefaultSubobject<USpaceManager>(TEXT("SpaceManager"));
}

void AJHSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AJHSGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AJHSGameMode::StartGame()
{
	StartStage(1);
}

void AJHSGameMode::StartStage(int32 Stage)
{
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	TObjectPtr<UTurretStateGroup> _turretStateGroup = _outGameState->GetTurretStateGroup();
	_turretStateGroup->SetInfiniteMagMode(true);
	_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Main, E_AMMO_TYPE::Bullet, nullptr);
	_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Right, E_AMMO_TYPE::Bullet, nullptr);
	_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Left, E_AMMO_TYPE::Missile, nullptr);
}