// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIPanelDriveSeat.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/DataStruct.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UUIPanelDriveSeat::OnOpen()
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

void UUIPanelDriveSeat::OnClose()
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

	FMaxCurrentData _maxCurrentData = Event->MaxCurrentData;

	switch (_maxCurrentData.DataType)
	{
		case E_DATA_TYPE::HP:
			UpdateSpaceShipUI(_maxCurrentData.CurrentValue, _maxCurrentData.MaxValue, TXT_SpaceShipHP, PROG_SpaceShipHP);
			break;

		case E_DATA_TYPE::Shield:
			UpdateSpaceShipUI(_maxCurrentData.CurrentValue, _maxCurrentData.MaxValue, TXT_SpaceShipShield, PROG_SpaceShipShield);
			break;

		case E_DATA_TYPE::Fuel:
			UpdateSpaceShipUI(_maxCurrentData.CurrentValue, _maxCurrentData.MaxValue, TXT_SpaceShipFuel, PROG_SpaceShipFuel);
			break;

		default:
			break;
	}
}

void UUIPanelDriveSeat::UpdateSpaceShipUI(float CurrentValue, float MaxValue, UTextBlock* TextBlock, UProgressBar* ProgressBar)
{
	if (TextBlock == nullptr || ProgressBar == nullptr)
		return;

	float _percent = MaxValue > 0.0f ? (CurrentValue / MaxValue) : 0.0f;
	ProgressBar->SetPercent(_percent);

	FString _text = FString::Printf(TEXT("%d / %d"), (int32)CurrentValue, (int32)MaxValue);
	TextBlock->SetText(FText::FromString(_text));
}