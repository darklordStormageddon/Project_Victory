// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelTurretSeat.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/Contant/ConstantLibrary.h"

bool UUIPanelTurretSeat::Initialize()
{
    if (!Super::Initialize())
        return false;

    LoadAmmoTypeTexture();
    return true;
}

void UUIPanelTurretSeat::RegisterEvent()
{
    _eventHandleOnChangeTurret = GetEventManager()->AddListener<UEventOnChangeTurretState>(
        [this](UEventOnChangeTurretState* Event)
        {
            OnChangeTurret(Event);
        }
    );
}

void UUIPanelTurretSeat::UnregisterEvent()
{
	if (_eventHandleOnChangeTurret.IsValid())
    {
        GetEventManager()->DelListener<UEventOnChangeTurretState>(_eventHandleOnChangeTurret);
        _eventHandleOnChangeTurret.Reset();
    }
}

void UUIPanelTurretSeat::OnChangeTurret(UEventOnChangeTurretState* Event)
{
    if (Event == nullptr)
        return;

    FTurretState _turretState = Event->TurretState;
    FTurretData _turretData = _turretState.TurretData;
    if (_turretState.TurretPosition == E_TURRET_POSITION::Main)
    {
        const float _progress = FMath::Clamp(_turretData.Mag.CurrentValue / _turretData.Mag.MaxValue, 0.f, 1.f);
        GetMainTurretMaterial()->SetScalarParameterValue(TEXT("Progress"), _progress);

        FLinearColor _lerpColor = FMath::Lerp(_leftAmmoColorZero, _leftAmmoColorMax, _progress);
        GetMainTurretMaterial()->SetVectorParameterValue(TEXT("Tint"), _lerpColor);

        TXT_LeftAmmo->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_turretData.Mag.CurrentValue)));
        return;
    }

    TObjectPtr<UTexture2D> _texture = _ammoTypeTextureMap.FindRef(_turretData.AmmoType);
    if (_turretState.TurretPosition == E_TURRET_POSITION::Left)
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

void UUIPanelTurretSeat::LoadAmmoTypeTexture()
{
    for (int32 i = 0; i < (int32)E_AMMO_TYPE::NONE; i++)
    {
        E_AMMO_TYPE _ammoType = (E_AMMO_TYPE)i;
        FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_AMMO_TYPE>(_ammoType);
        FString _texturePath = ConstantLibrary::Resource.Image.AMMO_FOLDER_PATH + _fileName + "." + _fileName;
        TObjectPtr<UTexture2D> _texture = LoadObject<UTexture2D>(nullptr, *_texturePath);
        if (_texture == nullptr)
        {
            UE_LOG(LogTemp, Error, TEXT("UIPanelTurretSeat: Texture is not found from [%s]"), *_texturePath);
        }

        _ammoTypeTextureMap.Add(_ammoType, _texture);
    }
}