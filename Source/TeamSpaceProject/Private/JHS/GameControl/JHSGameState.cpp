// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

AJHSGameState::AJHSGameState()
{
	_spaceShipData.Hp.DataType = E_DATA_TYPE::HP;
	_spaceShipData.Shield.DataType = E_DATA_TYPE::Shield;
	_spaceShipData.Fuel.DataType = E_DATA_TYPE::Fuel;
}

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

void AJHSGameState::ChangeSpaceShipData(FMaxCurrentData* OriginalData, float CurrentValue)
{
	ChangeSpaceShipData(OriginalData, CurrentValue, OriginalData->MaxValue);
}

void AJHSGameState::ChangeSpaceShipData(FMaxCurrentData* OriginalData, float CurrentValue, float MaxValue)
{
	// FMaxCurrentData 원본 데이터 참조
	OriginalData->CurrentValue = CurrentValue;
	OriginalData->MaxValue = MaxValue;
	if (OriginalData->CurrentValue > OriginalData->MaxValue)
	{
		OriginalData->CurrentValue = OriginalData->MaxValue;
	}
	if (OriginalData->CurrentValue < 0.0f)
	{
		OriginalData->CurrentValue = 0.0f;
	}

	UEventManager* _eventManager = GetEventManager();
	if (_eventManager == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameState: EventManager is nullptr, skipping event execution"));
		return;
	}

	UEventOnChangeSpaceShipData* _event = NewObject<UEventOnChangeSpaceShipData>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameState: Failed to create UEventOnChangeSpaceShipData"));
		return;
	}

	_event->MaxCurrentData = *OriginalData;
	_eventManager->ExecuteEvent<UEventOnChangeSpaceShipData>(_event);
}

void AJHSGameState::SendCurrentDataEvent()
{
	ChangeSpaceShipData(&_spaceShipData.Hp, _spaceShipData.Hp.CurrentValue, _spaceShipData.Hp.MaxValue);
	ChangeSpaceShipData(&_spaceShipData.Shield, _spaceShipData.Shield.CurrentValue, _spaceShipData.Shield.MaxValue);
	ChangeSpaceShipData(&_spaceShipData.Fuel, _spaceShipData.Fuel.CurrentValue, _spaceShipData.Fuel.MaxValue);
}

void AJHSGameState::RepairSpaceShip()
{
	ChangeSpaceShipData(&_spaceShipData.Hp, _spaceShipData.Hp.MaxValue);
	ChangeSpaceShipData(&_spaceShipData.Shield, _spaceShipData.Shield.MaxValue);
	ChangeSpaceShipData(&_spaceShipData.Fuel, _spaceShipData.Fuel.MaxValue);
}

void AJHSGameState::DecreaseSpaceShipData(E_DATA_TYPE DataType, float DecreaseValue)
{
	switch (DataType)
	{
		case E_DATA_TYPE::HP:
			ChangeSpaceShipData(&_spaceShipData.Hp, _spaceShipData.Hp.CurrentValue - DecreaseValue);
			break;

		case E_DATA_TYPE::Shield:
			ChangeSpaceShipData(&_spaceShipData.Shield, _spaceShipData.Shield.CurrentValue - DecreaseValue);
			break;

		case E_DATA_TYPE::Fuel:
			ChangeSpaceShipData(&_spaceShipData.Fuel, _spaceShipData.Fuel.CurrentValue - DecreaseValue);
			break;

		default:
			break;
	}
}

void AJHSGameState::RepairShield(float RepairShieldValue)
{
	ChangeSpaceShipData(&_spaceShipData.Shield, _spaceShipData.Shield.CurrentValue + RepairShieldValue);
}