// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelDriveSeat.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/DataStruct.h"

void UUIPanelDriveSeat::RegisterEvent()
{
	// 이벤트 리스너 등록
	UEventManager* _outEventManager = nullptr;
	if (UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
	{
		_eventHandle = _outEventManager->AddListener<UEventOnChangeSpaceShipData>(
			[this](UEventOnChangeSpaceShipData* Event)
			{
				OnChangeSpaceShipData(Event);
			}
		);
	}

	AJHSGameState* _outGameState = nullptr;
	if (UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_outGameState->SendCurrentDataEvent();
}

void UUIPanelDriveSeat::UnregisterEvent()
{
	// 이벤트 리스너 제거
	UEventManager* _eventManager = nullptr;
	if (UStaticFunctionLibrary::TryGetEventManager(_eventManager))
	{
		if (_eventHandle.IsValid())
		{
			_eventManager->DelListener<UEventOnChangeSpaceShipData>(_eventHandle);
			_eventHandle.Reset();
		}
	}
}

void UUIPanelDriveSeat::OnChangeSpaceShipData(UEventOnChangeSpaceShipData* Event)
{
	if (Event == nullptr)
		return;

	FSpaceShipData _maxCurrentData = Event->SpaceShipDataData;

	switch (_maxCurrentData.DataType)
	{
		case E_SPACE_SHIP_DATA_TYPE::HP:
			SetProgressBarUI(_maxCurrentData.Values.CurrentValue, _maxCurrentData.Values.MaxValue, PROG_SpaceShipHP, TXT_SpaceShipHP);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Shield:
			SetProgressBarUI(_maxCurrentData.Values.CurrentValue, _maxCurrentData.Values.MaxValue, PROG_SpaceShipShield, TXT_SpaceShipShield);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Fuel:
			SetProgressBarUI(_maxCurrentData.Values.CurrentValue, _maxCurrentData.Values.MaxValue, PROG_SpaceShipFuel, TXT_SpaceShipFuel);
			break;

		default:
			break;
	}
}