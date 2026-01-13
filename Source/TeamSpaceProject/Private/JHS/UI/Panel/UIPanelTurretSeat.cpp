// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelTurretSeat.h"
#include "JHS/Event/EventManager.h"

void UUIPanelTurretSeat::RegisterEvent()
{
    _eventHandle = GetEventManager()->AddListener<UEventOnChangeTurretAmmo>(
        [this](UEventOnChangeTurretAmmo* Event)
        {
            OnChangeTurretAmmo(Event);
        }
    );
}

void UUIPanelTurretSeat::UnregisterEvent()
{
	if (_eventHandle.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeTurretAmmo>(_eventHandle);
        _eventHandle.Reset();
    }
}

void UUIPanelTurretSeat::OnChangeTurretAmmo(UEventOnChangeTurretAmmo* Event)
{
    if (Event == nullptr)
        return;

    if (_leftAmmoMID == nullptr)
    {
        UMaterialInterface* _baseMat = Cast<UMaterialInterface>(IMG_LeftAmmo->GetBrush().GetResourceObject());
        if (!_baseMat)
        {
            UE_LOG(LogTemp, Error, TEXT("UIPanelTurretSeat: Material is not found"));
            return;
        }
    
        _leftAmmoMID = UMaterialInstanceDynamic::Create(_baseMat, this);
        IMG_LeftAmmo->SetBrushFromMaterial(_leftAmmoMID);
    }

    FMaxCurrentData _ammo = Event->Ammo;
    
    // Progress
    const float _progress = FMath::Clamp(_ammo.CurrentValue / _ammo.MaxValue, 0.f, 1.f);
	_leftAmmoMID->SetScalarParameterValue(TEXT("Progress"), _progress);

    // Color
    FLinearColor _lerpColor = FMath::Lerp(_leftAmmoColorZero, _leftAmmoColorMax, _progress);
    _leftAmmoMID->SetVectorParameterValue(TEXT("Tint"), _lerpColor);

    // Text
    TXT_LeftAmmo->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_ammo.CurrentValue)));
}