// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/CommonInfo/UIPanelCommonInfo.h"
#include "JHS/UI/Panel/CommonInfo/PlayerInfoRow.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"

void UUIPanelCommonInfo::RegisterEvent()
{
	_eventHandle = GetEventManager()->AddListener<UEventOnChangePlayerRadiation>(
		[this](UEventOnChangePlayerRadiation* Event)
		{
			OnChangePlayerRadiation(Event);
		}
	);
}

void UUIPanelCommonInfo::UnregisterEvent()
{
	if (_eventHandle.IsValid())
	{
		GetEventManager()->DelListener<UEventOnChangePlayerRadiation>(_eventHandle);
		_eventHandle.Reset();
	}
}

void UUIPanelCommonInfo::InitializeUI(int32 PlayerNum)
{
	if (!Plate_RadiationDose)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Plate_RadiationDose or WBP_PlayerInfoRow is nullptr"));
		return;
	}

	ClearDynamicRows();

	UClass* _widgetClass = WBP_PlayerInfoRow->GetClass();
	if (!_widgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to get widget class from WBP_PlayerInfoRow"));
		return;
	}

	APlayerController* _playerController = GetOwningPlayer();
	if (!_playerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to get PlayerController"));
		return;
	}

	for (int32 _playerIndex = PlayerNum - 1; _playerIndex >= 0; --_playerIndex)
	{
		UPlayerInfoRow* _row = CreateWidget<UPlayerInfoRow>(_playerController, _widgetClass);
		if (!_row)
		{
			UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to create widget for player %d"), _playerIndex);
			continue;
		}

		_row->InitializeRaw(_playerIndex + 1);

		_row->SetVisibility(ESlateVisibility::Visible);
		Plate_RadiationDose->AddChild(_row);

		_playerRadiationDoseMap.Add(_playerIndex, _row);

		//UE_LOG(LogTemp, Warning, TEXT("UIPanelPlayerFPS: Created and added widget for player %d"), _playerIndex);
	}
}

void UUIPanelCommonInfo::ClearDynamicRows()
{
	if (!Plate_RadiationDose)
	{
		return;
	}

	for (int32 _i = Plate_RadiationDose->GetChildrenCount() - 1; _i >= 1; --_i)
	{
		Plate_RadiationDose->RemoveChildAt(_i);
	}
}

void UUIPanelCommonInfo::OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event)
{
	if (Event == nullptr)
		return;

	FPlayerStateData _maxCurrentData = Event->PlayerStateData;

	_playerRadiationDoseMap[_maxCurrentData.PlayerIndex]->UpdatePlayerRadiationDose(_maxCurrentData.Radiation);

}