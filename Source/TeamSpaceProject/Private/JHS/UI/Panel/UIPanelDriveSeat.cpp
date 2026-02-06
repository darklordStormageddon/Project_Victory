// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelDriveSeat.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

void UUIPanelDriveSeat::RegisterEvent()
{
	_eventHandle = GetEventManager()->AddListener<UEventOnChangeSpaceShipData>(
		[this](UEventOnChangeSpaceShipData* Event)
		{
			OnChangeSpaceShipData(Event);
		}
	);
}

void UUIPanelDriveSeat::UnregisterEvent()
{
	if (_eventHandle.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeSpaceShipData>(_eventHandle);
        _eventHandle.Reset();
    }
}

void UUIPanelDriveSeat::OnChangeSpaceShipData(UEventOnChangeSpaceShipData* Event)
{
	if (Event == nullptr)
		return;

	FSpaceShipData _maxCurrentData = Event->SpaceShipDataData;
	FMaxCurrentData _value = _maxCurrentData.Data.Value;

	switch (_maxCurrentData.DataType)
	{
		case E_SPACE_SHIP_DATA_TYPE::HP:
			SetProgressBarUI(_value.CurrentValue, _value.MaxValue, PROG_SpaceShipHP, TXT_SpaceShipHP, true);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Shield:
			SetProgressBarUI(_value.CurrentValue, _value.MaxValue, PROG_SpaceShipShield, TXT_SpaceShipShield, true);
			break;

		case E_SPACE_SHIP_DATA_TYPE::Fuel:
			SetProgressBarUI(_value.CurrentValue, _value.MaxValue, PROG_SpaceShipFuel, TXT_SpaceShipFuel, true);
			break;

		default:
			break;
	}
}