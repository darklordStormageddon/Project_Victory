// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Turret/UIPanelTurretSeat.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/UI/Material/CircleProgressBar.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UUIPanelTurretSeat::UUIPanelTurretSeat(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // CircleProgressBar 컴포넌트 생성 (에디터에서 보이도록)
    _circleProgressBar = CreateDefaultSubobject<UCircleProgressBar>(TEXT("CircleProgressBar"));
}

void UUIPanelTurretSeat::NativePreConstruct()
{
    Super::NativePreConstruct();

    // 에디터 미리보기를 위한 CircleProgressBar 초기화
    // EditDefaultsOnly 값이 로드된 후, BindWidget이 바인딩된 후 실행됨
    if (_circleProgressBar && IMG_LeftAmmo)
    {
        _circleProgressBar->InitializeCircleProgressBar(this, IMG_LeftAmmo, _initTexture, _isClockWise);
    }
}

void UUIPanelTurretSeat::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    AJHSGameState* _outGameState = nullptr;
    if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
        return;
    
    _turretStateGroup = _outGameState->GetTurretStateGroup();
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
    FMaxCurrentData _mag = _turretData.Mag.Value;
    if (_turretPosition == E_TURRET_POSITION::Main)
    {
        if (_circleProgressBar)
        {
            const float _progress = FMath::Clamp(_mag.CurrentValue / _mag.MaxValue, 0.f, 1.f);
            _circleProgressBar->SetProgress(_progress);

            FLinearColor _lerpColor = FMath::Lerp(_leftAmmoColorZero, _leftAmmoColorMax, _progress);
            _circleProgressBar->SetTint(_lerpColor);
        }

        TXT_LeftAmmo->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_mag.CurrentValue)));
        return;
    }

    FAmmoData* _outAmmoData = nullptr;
    if (!_turretStateGroup->TryGetAmmoData(_turretData.AmmoType, _outAmmoData))
        return;
    
    TObjectPtr<UTexture2D> _texture = _outAmmoData->AmmoImage;
    if (_turretPosition == E_TURRET_POSITION::Left)
    {
        IMG_LeftTurret->SetBrushFromTexture(_texture);
        SetProgressBarUI(_mag.CurrentValue, _mag.MaxValue, PROG_LeftTurretAmmo, TXT_LeftTurretAmmo, true);
    }
    else
    {
        IMG_RightTurret->SetBrushFromTexture(_texture);
        SetProgressBarUI(_mag.CurrentValue, _mag.MaxValue, PROG_RightTurretAmmo, TXT_RightTurretAmmo, true);
    }
}