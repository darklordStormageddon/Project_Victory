// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Event/EventManager.h"

AJHSGameMode::AJHSGameMode()
{
	_spaceObjectManager = CreateDefaultSubobject<USpaceObjectManager>(TEXT("SpaceObjectManager"));
	_uiManager = CreateDefaultSubobject<UUIManager>(TEXT("UIManager"));
	_eventManager = CreateDefaultSubobject<UEventManager>(TEXT("EventManager"));
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

ASpaceStation* AJHSGameMode::GetSpaceStation()
{
	// 없으면 스캔해서 찾기
	if (_spaceStation == nullptr)
	{
		UWorld* _world = GetWorld();
		if (!_world)
		{
			UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: _world is nullptr"));
			return nullptr;
		}

		TArray<AActor*> _foundSpaceStationArray;
		UGameplayStatics::GetAllActorsOfClass(_world, ASpaceStation::StaticClass(), _foundSpaceStationArray);
		if (_foundSpaceStationArray.Num() > 0)
		{
			_spaceStation = Cast<ASpaceStation>(_foundSpaceStationArray[0]);
		}
	}

	return _spaceStation;
}
