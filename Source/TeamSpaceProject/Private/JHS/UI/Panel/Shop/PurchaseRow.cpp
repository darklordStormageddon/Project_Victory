// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Shop/PurchaseRow.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPurchaseRow::UpdateRow(FPurchaseData PurchaseData)
{
    IMG_Icon->SetBrushFromTexture(PurchaseData.Image);
    TXT_Description->SetText(FText::FromString(PurchaseData.Description));

    const float _currentValue = PurchaseData.Value.CurrentValue;
    TXT_CurrentValue->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_currentValue)));
    const float _nextValue = _currentValue * (1.0f + PurchaseData.IncreasePerValue * 0.01f);
    TXT_NextValue->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_nextValue)));

    TXT_PurchaseDollar->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)PurchaseData.PurchaseDollar)));
}