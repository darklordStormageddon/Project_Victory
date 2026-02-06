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

void AJHSGameMode::StartGame(AActor* Caller)
{
	// 서버가 아니면 실행하지 않음
	if (Caller == nullptr || !Caller->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameMode::StartGame - Not called from server"));
		return;
	}

	// 이미 게임이 시작되었으면 중복 실행 방지
	if (_isGameStarted)
		return;

	_isGameStarted = true;
	StartStage(1);
}

void AJHSGameMode::StartStage(int32 Stage)
{
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	TObjectPtr<UTurretStateGroup> _turretStateGroup = _outGameState->GetTurretStateGroup();
	_turretStateGroup->SetInfiniteMagMode(true);
	_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Main, E_AMMO_TYPE::Bullet);
	//_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Right, E_AMMO_TYPE::Bullet);
	//_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Left, E_AMMO_TYPE::Missile);
}