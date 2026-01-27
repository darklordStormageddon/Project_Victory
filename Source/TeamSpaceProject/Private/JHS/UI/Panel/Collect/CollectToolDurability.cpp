// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Collect/CollectToolDurability.h"
#include "JHS/UI/Material/CircleProgressBar.h"
#include "Components/Image.h"

UCollectToolDurability::UCollectToolDurability(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    _circleProgressBar = CreateDefaultSubobject<UCircleProgressBar>(TEXT("CircleProgressBar"));
}

void UCollectToolDurability::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (_circleProgressBar && IMG_Durability)
    {
        _circleProgressBar->InitializeCircleProgressBar(this, IMG_Durability, _initTexture, false);
    }
}

void UCollectToolDurability::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UCollectToolDurability::SetDurabilityProgress(float Progress, TObjectPtr<UTexture2D> ToolImage)
{
    _circleProgressBar->SetProgress(Progress);
    IMG_Icon->SetBrushFromSoftTexture(ToolImage);
}
