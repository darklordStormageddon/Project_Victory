// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Turret/UIPanelTurretMagReload.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/CommonEnums.h"

void UUIPanelTurretMagReload::RegisterEvent()
{
    _eventHandleOnChangeTurret = GetEventManager()->AddListener<UEventOnChangeTurretState>(
        [this](UEventOnChangeTurretState* Event)
        {
            OnChangeTurret(Event);
        }
    );
}

void UUIPanelTurretMagReload::UnregisterEvent()
{
    if (_eventHandleOnChangeTurret.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeTurretState>(_eventHandleOnChangeTurret);
        _eventHandleOnChangeTurret.Reset();
    }
}

void UUIPanelTurretMagReload::OnChangeTurret(UEventOnChangeTurretState* Event)
{
    if (Event == nullptr)
        return;
    
    FTurretState _turretState = Event->TurretState;
    if (_turretState.TurretPosition != _turretPosition)
        return;

    SetProgressBarUI(_turretState.TurretData.Mag.CurrentValue, _turretState.TurretData.Mag.MaxValue, PROG_TurretMag, TXT_TurretMag, false);
}

void UUIPanelTurretMagReload::Initialize(E_TURRET_POSITION TurretPosition)
{
    _turretPosition = TurretPosition;
    TXT_TurretPosition->SetText(FText::FromString(FString::Printf(TEXT("%s"), *CommonEnums::GetEnum2FString<E_TURRET_POSITION>(_turretPosition))));
}