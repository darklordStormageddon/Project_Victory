// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/CommonInfo/UIPanelCommonInfo.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/Event/EventManager.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

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

void UUIPanelCommonInfo::InitializeUI(TObjectPtr<UTexture2D> RadiationImage)
{
	if (IMG_RadiationDose)
	{
		IMG_RadiationDose->SetBrushFromTexture(RadiationImage);
	}
}

void UUIPanelCommonInfo::OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event)
{
	if (Event == nullptr)
		return;

	const FPlayerStateData& _playerStateData = Event->PlayerStateData;
	const FMaxCurrentData& _radiationData = _playerStateData.Radiation;

	if (PROG_RadiationDose)
	{
		SetProgressBarUI(_radiationData.CurrentValue, _radiationData.MaxValue, PROG_RadiationDose);
	}
}