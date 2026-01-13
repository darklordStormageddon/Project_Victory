// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/CommonInfo/UIPanelCommonInfo.h"
#include "JHS/UI/Panel/CommonInfo/PlayerInfoRow.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"

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

void UUIPanelCommonInfo::OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event)
{
	if (Event == nullptr)
		return;

	if (_playerRadiationDoseMap.Num() <= 0)
	{
		AJHSGameState* _outGameState = nullptr;
		if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
			return;

		BuildRows(_outGameState->GetPlayerCount());
	}

	FPlayerStateData _maxCurrentData = Event->PlayerStateData;

	_playerRadiationDoseMap[_maxCurrentData.PlayerIdx]->UpdatePlayerRadiationDose(_maxCurrentData.Radiation);

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

void UUIPanelCommonInfo::BuildRows(int32 InPlayerCount)
{
	if (!Plate_RadiationDose)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Plate_RadiationDose or WBP_PlayerInfoRow is nullptr"));
		return;
	}

	ClearDynamicRows();

	// WBP_PlayerInfoRow�� ���� Ŭ���� �������� (��������Ʈ Ŭ������ �� ����)
	UClass* _widgetClass = WBP_PlayerInfoRow->GetClass();
	if (!_widgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to get widget class from WBP_PlayerInfoRow"));
		return;
	}

	// PlayerController �������� (CreateWidget�� �ʿ�)
	APlayerController* _playerController = GetOwningPlayer();
	if (!_playerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to get PlayerController"));
		return;
	}
	for (int32 _playerIndex = InPlayerCount - 1; _playerIndex >= 0; --_playerIndex)
	{
		// GetClass()�� ����Ͽ� ��������Ʈ Ŭ������ �ùٸ��� ó��
		UPlayerInfoRow* _row = CreateWidget<UPlayerInfoRow>(_playerController, _widgetClass);
		if (!_row)
		{
			UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to create widget for player %d"), _playerIndex);
			continue;
		}

		// ���� �ʱ�ȭ
		_row->InitializeRaw(_playerIndex + 1);

		// Visibility Ȯ�� �� ����
		_row->SetVisibility(ESlateVisibility::Visible);
		Plate_RadiationDose->AddChild(_row);

		_playerRadiationDoseMap.Add(_playerIndex, _row);

		UE_LOG(LogTemp, Warning, TEXT("UIPanelPlayerFPS: Created and added widget for player %d"), _playerIndex);
	}
}