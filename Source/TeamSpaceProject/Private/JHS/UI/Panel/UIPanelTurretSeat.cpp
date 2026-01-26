// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelTurretSeat.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/UI/Material/CircleProgressBar.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUIPanelTurretSeat::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    AJHSGameState* _outGameState = nullptr;
    if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
        return;
    
    _containerStateGroup = _outGameState->GetContainerStateGroup();

    // CircleProgressBar 초기화
    _circleProgressBar = NewObject<UCircleProgressBar>(this);
    if (_circleProgressBar && IMG_LeftAmmo)
    {
        _circleProgressBar->InitializeCircleProgressBar(this, IMG_LeftAmmo, _initTexture, _isClockWise);
    }
}

void UUIPanelTurretSeat::RegisterEvent()
{
    _eventHandleOnChangeTurret = GetEventManager()->AddListener<UEventOnChangeTurretData>(
        [this](UEventOnChangeTurretData* Event)
        {
            OnChangeTurret(Event);
        }
    );
}

void UUIPanelTurretSeat::UnregisterEvent()
{
	if (_eventHandleOnChangeTurret.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeTurretData>(_eventHandleOnChangeTurret);
        _eventHandleOnChangeTurret.Reset();
    }
}

void UUIPanelTurretSeat::OnChangeTurret(UEventOnChangeTurretData* Event)
{
    if (Event == nullptr)
        return;

    E_TURRET_POSITION _turretPosition = Event->TurretPosition;
    FTurretData _turretData = Event->TurretData;
    if (_turretPosition == E_TURRET_POSITION::Main)
    {
        if (_circleProgressBar)
        {
            const float _progress = FMath::Clamp(_turretData.Mag.CurrentValue / _turretData.Mag.MaxValue, 0.f, 1.f);
            _circleProgressBar->SetProgress(_progress);

            FLinearColor _lerpColor = FMath::Lerp(_leftAmmoColorZero, _leftAmmoColorMax, _progress);
            _circleProgressBar->SetTint(_lerpColor);
        }

        TXT_LeftAmmo->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_turretData.Mag.CurrentValue)));
        return;
    }

    FAmmoData* _outAmmoData = nullptr;
    if (!_containerStateGroup->TryGetAmmoData(_turretData.AmmoType, _outAmmoData))
        return;
    
    TObjectPtr<UTexture2D> _texture = _outAmmoData->AmmoImage;
    if (_turretPosition == E_TURRET_POSITION::Left)
    {
        IMG_LeftTurret->SetBrushFromTexture(_texture);
        SetProgressBarUI(_turretData.Mag.CurrentValue, _turretData.Mag.MaxValue, PROG_LeftTurretAmmo, TXT_LeftTurretAmmo, true);
    }
    else
    {
        IMG_RightTurret->SetBrushFromTexture(_texture);
        SetProgressBarUI(_turretData.Mag.CurrentValue, _turretData.Mag.MaxValue, PROG_RightTurretAmmo, TXT_RightTurretAmmo, true);
    }
}