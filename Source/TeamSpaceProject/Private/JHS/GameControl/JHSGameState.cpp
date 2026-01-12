// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
#include "JHS/UI/Panel/CommonInfo/UIPanelCommonInfo.h"

AJHSGameState::AJHSGameState()
{
	_spaceShipState.Hp.DataType = E_SPACE_SHIP_DATA_TYPE::HP;
	_spaceShipState.Shield.DataType = E_SPACE_SHIP_DATA_TYPE::Shield;
	_spaceShipState.Fuel.DataType = E_SPACE_SHIP_DATA_TYPE::Fuel;
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
	for (FPlayerStateData _playerState : PlayerStateArray)
	{
		_playerState.Radiation.MaxValue = _maxPlayerRadiation;
		_playerState.Radiation.CurrentValue = 0.0f;

		_playerStateMap.Add(_playerState.PlayerIdx, _playerState);
	}

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	UUIBase* _uiCommonInfo = _outUIManager->OpenUI(E_UI_TYPE::UIPanelCommonInfo);
	Cast<UUIPanelCommonInfo>(_uiCommonInfo)->InitializeUI();
}

void AJHSGameState::SendCurrentDataEvent()
{
	ChangeSpaceShipData(&_spaceShipState.Hp, _spaceShipState.Hp.Values.CurrentValue, _spaceShipState.Hp.Values.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Values.CurrentValue, _spaceShipState.Shield.Values.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.Fuel, _spaceShipState.Fuel.Values.CurrentValue, _spaceShipState.Fuel.Values.MaxValue);
}

#pragma region SpaceShip
void AJHSGameState::RepairSpaceShip()
{
	ChangeSpaceShipData(&_spaceShipState.Hp, _spaceShipState.Hp.Values.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Values.MaxValue);
	ChangeSpaceShipData(&_spaceShipState.Fuel, _spaceShipState.Fuel.Values.MaxValue);
}

void AJHSGameState::DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE DataType, float DecreaseValue)
{
	switch (DataType)
	{
		case E_SPACE_SHIP_DATA_TYPE::HP:
			ChangeSpaceShipData(&_spaceShipState.Hp, _spaceShipState.Hp.Values.CurrentValue - DecreaseValue);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Shield:
			ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Values.CurrentValue - DecreaseValue);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Fuel:
			ChangeSpaceShipData(&_spaceShipState.Fuel, _spaceShipState.Fuel.Values.CurrentValue - DecreaseValue);
			break;

		default:
			break;
	}
}

void AJHSGameState::RepairShield(float RepairShieldValue)
{
	ChangeSpaceShipData(&_spaceShipState.Shield, _spaceShipState.Shield.Values.CurrentValue + RepairShieldValue);
}

void AJHSGameState::ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue)
{
	ChangeSpaceShipData(OriginalData, CurrentValue, OriginalData->Values.MaxValue);
}

void AJHSGameState::ChangeSpaceShipData(FSpaceShipData* OriginalData, float CurrentValue, float MaxValue)
{
	// FMaxCurrentData 원본 데이터 참조
	OriginalData->Values.CurrentValue = CurrentValue;
	OriginalData->Values.MaxValue = MaxValue;
	if (OriginalData->Values.CurrentValue > OriginalData->Values.MaxValue)
	{
		OriginalData->Values.CurrentValue = OriginalData->Values.MaxValue;
	}
	if (OriginalData->Values.CurrentValue < 0.0f)
	{
		OriginalData->Values.CurrentValue = 0.0f;
	}

	UEventOnChangeSpaceShipData* _event = NewObject<UEventOnChangeSpaceShipData>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameState: Failed to create UEventOnChangeSpaceShipData"));
		return;
	}

	_event->SpaceShipDataData = *OriginalData;
	GetEventManager()->ExecuteEvent<UEventOnChangeSpaceShipData>(_event);
}
#pragma endregion SpaceShip

#pragma region Player State
void AJHSGameState::IncreasePlayerRadiation(int32 PlayerIdx, float IncreaseValue)
{
	// 원본 데이터를 직접 수정
	FPlayerStateData* _playerState = _playerStateMap.Find(PlayerIdx);
	_playerState->Radiation.CurrentValue += IncreaseValue;
	if (_playerState->Radiation.CurrentValue > _playerState->Radiation.MaxValue)
	{
		_playerState->Radiation.CurrentValue = _playerState->Radiation.MaxValue;
	}

	UEventOnChangePlayerRadiation* _event = NewObject<UEventOnChangePlayerRadiation>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameState: Failed to create UEventOnChangePlayerRadiation"));
		return;
	}

	_event->PlayerStateData = *_playerState;
	GetEventManager()->ExecuteEvent<UEventOnChangePlayerRadiation>(_event);
}
#pragma endregion Player State

TObjectPtr<UEventManager> AJHSGameState::GetEventManager()
{
	if (_cachedEventManager == nullptr)
	{
		AJHSGameMode* _outGameMode = nullptr;
		if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
			return nullptr;

		_cachedEventManager = _outGameMode->GetEventManager();
	}

	if (_cachedEventManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameState: Failed to get EventManager"));
		return nullptr;
	}

	return _cachedEventManager;
}