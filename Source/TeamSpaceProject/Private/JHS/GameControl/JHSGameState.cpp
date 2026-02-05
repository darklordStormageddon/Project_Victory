// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/Event/EventManager.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"

AJHSGameState::AJHSGameState()
{
	_rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = _rootComponent;

	_spaceShipStateGroup = CreateDefaultSubobject<USpaceShipStateGroup>(TEXT("SpaceShipStateGroup"));

	_playerStateGroup = CreateDefaultSubobject<UPlayerStateGroup>(TEXT("PlayerStateGroup"));

	_collectStateGroup = CreateDefaultSubobject<UCollectStateGroup>(TEXT("CollectStateGroup"));

	_containerStateGroup = CreateDefaultSubobject<UContainerStateGroup>(TEXT("ContainerStateGroup"));

	_turretStateGroup = CreateDefaultSubobject<UTurretStateGroup>(TEXT("TurretStateGroup"));
	for (int32 i = 0; i < ((int32)E_AMMO_TYPE::Missile + 1); i++)
	{
		FAmmoData _ammoData;
		_ammoData.AmmoType = (E_AMMO_TYPE)i;
		_initAmmoDataArray.Add(_ammoData);
	}
}

void AJHSGameState::BeginPlay()
{
	Super::BeginPlay();

	TArray<FPlayerStateData> _playerStateArray;
	for (int32 i = 0; i < _testPlayerCount; i++)
	{
		FPlayerStateData _new;
		_new.PlayerUID = i;
		_new.PlayerIdx = i;
		_playerStateArray.Add(_new);
	}

	InitializeGameState(_playerStateArray);
}

void AJHSGameState::InitializeGameState(TArray<FPlayerStateData> PlayerStateArray)
{
	_spaceShipStateGroup->InitializeSpaceShipState(this, _initSpaceShipState);

	_playerStateGroup->InitializePlayerState(this, PlayerStateArray, _maxPlayerRadiation);

	_collectStateGroup->InitializeCollectState(this);

	_containerStateGroup->InitializeContainerState(this, _initContainerState, _initAmmoDataArray);
	
	_turretStateGroup->InitializeTurretState(this);

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_outUIManager->OpenUI(E_UI_TYPE::UIPanelCommonInfo);
}

void AJHSGameState::SendCurrentDataEvent()
{
	_spaceShipStateGroup->UpdateSpaceShipState();

	_playerStateGroup->UpdatePlayerState();

	_collectStateGroup->UpdateCollectState();

	_containerStateGroup->UpdateContainerState();

	_turretStateGroup->UpdateTurretState();
}

TObjectPtr<UEventManager> AJHSGameState::GetEventManager()
{
	if (_cachedEventManager == nullptr)
	{
		UEventManager* _outEventManager = nullptr;
		if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
			return nullptr;

		_cachedEventManager = _outEventManager;
	}

	if (_cachedEventManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameState: Failed to get EventManager"));
		return nullptr;
	}

	return _cachedEventManager;
}