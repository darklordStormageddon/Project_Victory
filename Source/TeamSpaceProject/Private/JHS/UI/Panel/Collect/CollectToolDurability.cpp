// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Collect/CollectToolDurability.h"
#include "JHS/UI/Material/CircleProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

UCollectToolDurability::UCollectToolDurability(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    _circleProgressBar = CreateDefaultSubobject<UCircleProgressBar>(TEXT("CircleProgressBar"));
}

void UCollectToolDurability::InitializeToolDurability()
{
    IMG_SelectHighlight->SetVisibility(ESlateVisibility::Hidden);
    if (_circleProgressBar && IMG_Durability)
    {
        _circleProgressBar->InitializeCircleProgressBar(this, IMG_Durability, _initTexture, false);
    }
}

void UCollectToolDurability::SetDurabilityProgress(TObjectPtr<UTexture2D> ToolImage, float Progress, bool IsVacuumTool)
{
    IMG_Icon->SetBrushFromSoftTexture(ToolImage);

    // Progress
    _circleProgressBar->SetProgress(Progress);

    // Color
    FLinearColor _lerpColor;
    float _lerpRate = 0.0f;
    if (Progress < 0.5f)
    {
        _lerpRate = Progress * 2.0f;
        _lerpColor = FMath::Lerp(_durabilityColorZero, _durabilityColorMiddle, _lerpRate);
    }
    else
    {
        _lerpRate = (Progress - 0.5f) * 2.0f;
        _lerpColor = FMath::Lerp(_durabilityColorMiddle, _durabilityColorMax, _lerpRate);
    }
    _circleProgressBar->SetTint(_lerpColor);

    // Text
    FString _text;
    if (IsVacuumTool)
    {
        _text = FString::Printf(TEXT(""));
    }
    else
    {
        const int32 _percent = (int32)(Progress * 100.0f);
        _text = FString::Printf(TEXT("%d%%"), _percent);
    }
    TXT_Durability->SetText(FText::FromString(_text));
}

void UCollectToolDurability::SelectTool(bool IsSelected)
{
    ESlateVisibility _visibility = IsSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    IMG_SelectHighlight->SetVisibility(_visibility);
}