// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Turret/UIPanelTurretMagReload.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/CommonEnums.h"

void UUIPanelTurretMagReload::RegisterEvent()
{
    _eventHandleOnChangeTurret = GetEventManager()->AddListener<UEventOnChangeTurretData>(
        [this](UEventOnChangeTurretData* Event)
        {
            OnChangeTurret(Event);
        }
    );
}

void UUIPanelTurretMagReload::UnregisterEvent()
{
    if (_eventHandleOnChangeTurret.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeTurretData>(_eventHandleOnChangeTurret);
        _eventHandleOnChangeTurret.Reset();
    }
}

void UUIPanelTurretMagReload::OnChangeTurret(UEventOnChangeTurretData* Event)
{
    if (Event == nullptr)
        return;
    
    E_TURRET_POSITION _eventTurretPosition = Event->TurretPosition;
    if (_eventTurretPosition != _turretPosition)
        return;

    FTurretData _turretData = Event->TurretData;
    SetProgressBarUI(_turretData.Mag.CurrentValue, _turretData.Mag.MaxValue, PROG_TurretMag, TXT_TurretMag, false);
}

void UUIPanelTurretMagReload::Initialize(E_TURRET_POSITION TurretPosition)
{
    _turretPosition = TurretPosition;
    TXT_TurretPosition->SetText(FText::FromString(FString::Printf(TEXT("%s"), *CommonEnums::GetEnum2FString<E_TURRET_POSITION>(_turretPosition))));
}