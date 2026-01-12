// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/PlayerFPS/UIPanelPlayerFPS.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/DataStruct.h"
#include "JHS/UI/PlayerFPS/PlayerInfoRow.h"

void UUIPanelPlayerFPS::RegisterEvent()
{
	UEventManager* _outEventManager = nullptr;
	if (UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
	{
		_eventHandle = _outEventManager->AddListener<UEventOnChangePlayerRadiation>(
			[this](UEventOnChangePlayerRadiation* Event)
			{
				OnChangePlayerRadiation(Event);
			}
		);
	}

	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_outGameState->SendCurrentDataEvent();
}

void UUIPanelPlayerFPS::UnregisterEvent()
{
	UEventManager* _eventManager = nullptr;
	if (UStaticFunctionLibrary::TryGetEventManager(_eventManager))
	{
		if (_eventHandle.IsValid())
		{
			_eventManager->DelListener<UEventOnChangePlayerRadiation>(_eventHandle);
			_eventHandle.Reset();
		}
	}
}

void UUIPanelPlayerFPS::OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event)
{
	if (Event == nullptr)
		return;

	FPlayerStateData _maxCurrentData = Event->PlayerStateData;

	_playerRadiationDoseMap[_maxCurrentData.PlayerIdx]->UpdatePlayerRadiationDose(_maxCurrentData.Radiation);

}

void UUIPanelPlayerFPS::InitializeUI()
{
	ChangeInteractable(E_INTERACT_TYPE::None);

	// Test
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	BuildRows(_outGameState->GetPlayerCount());
}

void UUIPanelPlayerFPS::ChangeInteractable(E_INTERACT_TYPE InteractType)
{
	if (Plate_Interact == nullptr)
		return;

	if (InteractType == E_INTERACT_TYPE::None)
	{
		Plate_Interact->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		Plate_Interact->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUIPanelPlayerFPS::ClearDynamicRows()
{
	if (!Plate_RadiationDose)
	{
		return;
	}

	// 0번은 TopSpacer라고 가정하고, 나머지는 제거
	// (TopSpacer 외에 고정 위젯이 더 있으면 "남길 인덱스/이름" 기준으로 필터링하세요.)
	for (int32 _i = Plate_RadiationDose->GetChildrenCount() - 1; _i >= 1; --_i)
	{
		Plate_RadiationDose->RemoveChildAt(_i);
	}
}

void UUIPanelPlayerFPS::BuildRows(int32 InPlayerCount)
{
	if (!Plate_RadiationDose || !WBP_PlayerInfoRow)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Plate_RadiationDose or WBP_PlayerInfoRow is nullptr"));
		return;
	}

	ClearDynamicRows();

	// WBP_PlayerInfoRow의 실제 클래스 가져오기 (블루프린트 클래스일 수 있음)
	UClass* _widgetClass = WBP_PlayerInfoRow->GetClass();
	if (!_widgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to get widget class from WBP_PlayerInfoRow"));
		return;
	}

	// PlayerController 가져오기 (CreateWidget에 필요)
	APlayerController* _playerController = GetOwningPlayer();
	if (!_playerController)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to get PlayerController"));
		return;
	}
	for (int32 _playerIndex = InPlayerCount - 1; _playerIndex >= 0; --_playerIndex)
	{
		// GetClass()를 사용하여 블루프린트 클래스도 올바르게 처리
		UPlayerInfoRow* _row = CreateWidget<UPlayerInfoRow>(_playerController, _widgetClass);
		if (!_row)
		{
			UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Failed to create widget for player %d"), _playerIndex);
			continue;
		}

		// 위젯 초기화
		_row->InitializeRaw(_playerIndex + 1);
		
		// Visibility 확인 및 설정
		_row->SetVisibility(ESlateVisibility::Visible);
		Plate_RadiationDose->AddChild(_row);

		_playerRadiationDoseMap.Add(_playerIndex, _row);
		
		UE_LOG(LogTemp, Warning, TEXT("UIPanelPlayerFPS: Created and added widget for player %d"), _playerIndex);
	}
}