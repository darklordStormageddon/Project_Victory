// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/CommonInfo/UIPanelCommonInfo.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/Event/EventManager.h"
#include "Components/HorizontalBox.h"
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
	GRP_RadiationDose->SetVisibility(ESlateVisibility::Hidden);

	if (IMG_RadiationDose)
	{
		IMG_RadiationDose->SetBrushFromTexture(RadiationImage);
	}
}

void UUIPanelCommonInfo::OnChangePlayerRadiation(UEventOnChangePlayerRadiation* Event)
{
	if (Event == nullptr)
		return;
	
	const int32& _callerAssignedPlayerId = Event->CallerAssignedPlayerId;

	if (!UStaticFunctionLibrary::CheckIsSelfClient(_callerAssignedPlayerId))
		return;

	const FMaxCurrentData& _radiationData = Event->RadiationData;

	if (PROG_RadiationDose)
	{
		SetProgressBarUI(_radiationData.CurrentValue, _radiationData.MaxValue, PROG_RadiationDose);
	}
}