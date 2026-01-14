// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelTurretSeat.h"
#include "JHS/Event/EventManager.h"

void UUIPanelTurretSeat::RegisterEvent()
{
    _eventHandleOnChangeTurret = GetEventManager()->AddListener<UEventOnChangeTurretData>(
        [this](UEventOnChangeTurretData* Event)
        {
            OnChangeTurret(Event);
        }
    );

    _eventHandleOnChangeTurretAmmo = GetEventManager()->AddListener<UEventOnChangeTurretAmmo>(
        [this](UEventOnChangeTurretAmmo* Event)
        {
            OnChangeTurretAmmo(Event);
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

    if (_eventHandleOnChangeTurretAmmo.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeTurretAmmo>(_eventHandleOnChangeTurretAmmo);
        _eventHandleOnChangeTurretAmmo.Reset();
    }
}

void UUIPanelTurretSeat::OnChangeTurret(UEventOnChangeTurretData* Event)
{
    if (Event == nullptr)
        return;

    FTurretData _turretData = Event->TurretData;
    if (_turretData.TurretPosition == E_TURRET_POSITION::Main)
    {
        const float _progress = FMath::Clamp(_turretData.Ammo.CurrentValue / _turretData.Ammo.MaxValue, 0.f, 1.f);
        GetMainTurretMaterial()->SetScalarParameterValue(TEXT("Progress"), _progress);

        FLinearColor _lerpColor = FMath::Lerp(_leftAmmoColorZero, _leftAmmoColorMax, _progress);
        GetMainTurretMaterial()->SetVectorParameterValue(TEXT("Tint"), _lerpColor);

        TXT_LeftAmmo->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_turretData.Ammo.CurrentValue)));
        return;
    }

    FColor _turretColor;
    switch (_turretData.AmmoType)
    {
        case E_AMMO_TYPE::Bullet:
            _turretColor = FColor::Purple;
            break;
        case E_AMMO_TYPE::Cannon:
            _turretColor = FColor::Red;
            break;
        case E_AMMO_TYPE::Missile:
            _turretColor = FColor::Green;
            break;
    }
    if (_turretData.TurretPosition == E_TURRET_POSITION::Left)
    {
        IMG_LeftTurret->SetBrushTintColor(_turretColor);
        SetProgressBarUI(_turretData.Ammo.CurrentValue, _turretData.Ammo.MaxValue, PROG_LeftTurretAmmo, TXT_LeftTurretAmmo);
    }
    else
    {
        IMG_RightTurret->SetBrushTintColor(_turretColor);
        SetProgressBarUI(_turretData.Ammo.CurrentValue, _turretData.Ammo.MaxValue, PROG_RightTurretAmmo, TXT_RightTurretAmmo);
    }
}

void UUIPanelTurretSeat::OnChangeTurretAmmo(UEventOnChangeTurretAmmo* Event)
{
    if (Event == nullptr)
        return;

    E_TURRET_POSITION _turretPosition = Event->TurretPosition;
    FMaxCurrentData _ammo = Event->Ammo;

    float _progress = 0.0f;
    FLinearColor _lerpColor = FLinearColor::Yellow;
    switch (_turretPosition)
    {
        case E_TURRET_POSITION::Main:
            _progress = FMath::Clamp(_ammo.CurrentValue / _ammo.MaxValue, 0.f, 1.f);
            GetMainTurretMaterial()->SetScalarParameterValue(TEXT("Progress"), _progress);

            _lerpColor = FMath::Lerp(_leftAmmoColorZero, _leftAmmoColorMax, _progress);
            GetMainTurretMaterial()->SetVectorParameterValue(TEXT("Tint"), _lerpColor);

            TXT_LeftAmmo->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_ammo.CurrentValue)));
            break;

        case E_TURRET_POSITION::Left:
            SetProgressBarUI(_ammo.CurrentValue, _ammo.MaxValue, PROG_LeftTurretAmmo, TXT_LeftTurretAmmo);
            break;

        case E_TURRET_POSITION::Right:
            SetProgressBarUI(_ammo.CurrentValue, _ammo.MaxValue, PROG_RightTurretAmmo, TXT_RightTurretAmmo);
            break;
    }
}

TObjectPtr<UMaterialInstanceDynamic> UUIPanelTurretSeat::GetMainTurretMaterial()
{
    if (_mainAmmoMID == nullptr)
    {
        UMaterialInterface* _baseMat = Cast<UMaterialInterface>(IMG_LeftAmmo->GetBrush().GetResourceObject());
        if (!_baseMat)
        {
            UE_LOG(LogTemp, Error, TEXT("UIPanelTurretSeat: Material is not found"));
            return nullptr;
        }
    
        _mainAmmoMID = UMaterialInstanceDynamic::Create(_baseMat, this);
        IMG_LeftAmmo->SetBrushFromMaterial(_mainAmmoMID);
    }

    return _mainAmmoMID;
}