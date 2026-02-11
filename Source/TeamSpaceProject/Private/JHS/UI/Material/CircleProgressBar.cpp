// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Material/CircleProgressBar.h"
#include "JHS/GameControl/Constant/ResourcePath.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UCircleProgressBar::UCircleProgressBar()
{
}

void UCircleProgressBar::InitializeCircleProgressBar(TObjectPtr<UObject> InOuter, TObjectPtr<UImage> Image, TObjectPtr<UTexture2D> Texture, bool IsClockWise)
{
    _inOuter = InOuter;
    _image = Image;

    // ResourcePath에서 머티리얼 경로 생성
    FResourceMaterial _resourceMaterial;
    FString _materialName = _resourceMaterial.MATERIAL_HEADER + _resourceMaterial.CIRCLE_PROGRESS_BAR;
    FString _materialPath = _resourceMaterial.MATERIAL_FORDER_PATH + _materialName + TEXT(".") + _materialName;
    
    // 머티리얼 로드
    UMaterialInterface* _baseMat = LoadObject<UMaterialInterface>(nullptr, *_materialPath);
    if (_baseMat && _image)
    {
        _MID = UMaterialInstanceDynamic::Create(_baseMat, _inOuter);
        _image->SetBrushFromMaterial(_MID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CircleProgressBar: Failed to load material from path: %s"), *_materialPath);
    }

    SetTexture(Texture);
    SetClockWise(IsClockWise);
}

void UCircleProgressBar::SetTexture(TObjectPtr<UTexture2D> Texture)
{
    _MID->SetTextureParameterValue(PARAMETER_TEXTURE, Texture);
}

void UCircleProgressBar::SetClockWise(bool IsClockWise)
{
    _MID->SetScalarParameterValue(PARAMETER_IS_CLOCK_WISE, IsClockWise ? 1.0f : 0.0f);
}

void UCircleProgressBar::SetProgress(float Progress)
{
    if (Progress <= 0.0f)
    {
        _MID->SetScalarParameterValue(PARAMETER_PROGRESS, 0.0f);
        return;
    }

    _MID->SetScalarParameterValue(PARAMETER_PROGRESS, Progress);
}

void UCircleProgressBar::SetTint(FLinearColor Tint)
{
    _MID->SetVectorParameterValue(PARAMETER_TINT, Tint);
}