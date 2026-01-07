// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIPanelDriveSeat.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/DataStruct.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UUIPanelDriveSeat::OnOpen()
{
	Super::OnOpen();

	// 이벤트 리스너 등록
	UEventManager* _eventManager = nullptr;
	if (UStaticFunctionLibrary::TryGetEventManager(_eventManager))
	{
		_eventHandle = _eventManager->AddListener<UEventOnChangeSpaceShipData>(
			[this](UEventOnChangeSpaceShipData* Event)
			{
				OnChangeSpaceShipData(Event);
			}
		);
	}
}

void UUIPanelDriveSeat::OnClose()
{
	Super::OnClose();

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

	FSpaceShipData _spaceShipData = Event->SpaceShipData;

	// HP 업데이트
	if (PROG_SpaceShipHP != nullptr)
	{
		float _hpPercent = _spaceShipData.MaxHP > 0.0f ? (_spaceShipData.CurrentHP / _spaceShipData.MaxHP) : 0.0f;
		PROG_SpaceShipHP->SetPercent(_hpPercent);
	}

	if (TXT_SpaceShipHP != nullptr)
	{
		FString _hpText = FString::Printf(TEXT("%.0f / %.0f"), _spaceShipData.CurrentHP, _spaceShipData.MaxHP);
		TXT_SpaceShipHP->SetText(FText::FromString(_hpText));
	}

	// Shield 업데이트
	if (PROG_SpaceShipShield != nullptr)
	{
		float _shieldPercent = _spaceShipData.MaxShield > 0.0f ? (_spaceShipData.CurrentShield / _spaceShipData.MaxShield) : 0.0f;
		PROG_SpaceShipShield->SetPercent(_shieldPercent);
	}

	if (TXT_SpaceShipShield != nullptr)
	{
		FString _shieldText = FString::Printf(TEXT("%.0f / %.0f"), _spaceShipData.CurrentShield, _spaceShipData.MaxShield);
		TXT_SpaceShipShield->SetText(FText::FromString(_shieldText));
	}

	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, FString::Printf(TEXT("Update UI")));
}

