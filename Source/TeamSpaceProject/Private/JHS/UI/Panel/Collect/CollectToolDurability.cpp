// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Collect/CollectToolDurability.h"
#include "JHS/UI/Material/CircleProgressBar.h"

void UCollectToolDurability::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // CircleProgressBar √ ±‚»≠
    _circleProgressBar = NewObject<UCircleProgressBar>(this);
    if (_circleProgressBar && IMG_Durability)
    {
        _circleProgressBar->InitializeCircleProgressBar(this, IMG_Durability, _initTexture, false);
    }
}