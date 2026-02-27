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

    FTurretData _turretData = Event->TurretData;
    SetProgressBarUI(_turretData.Mag.Value.CurrentValue, _turretData.Mag.Value.MaxValue, PROG_TurretMag, TXT_TurretMag, false);
}

void UUIPanelTurretMagReload::InitializeMagReload()
{
    TXT_TurretPosition->SetText(FText::FromString(FString::Printf(TEXT("Ammo"))));
}