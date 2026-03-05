// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Shop/PurchaseRow.h"
#include "JHS/GameControl/ShopManager.h"
#include "JHS/UI/Panel/Shop/PurchaseCategory.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "JHS/UI/Interact/InteractableButton.h"

void UPurchaseRow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Purchase != nullptr)
	{
		FString _buttonLabel = TEXT("구매");
		BTN_Purchase->InitializeButton(_buttonLabel);
		BTN_Purchase->ClickedEnter.AddDynamic(this, &UPurchaseRow::OnClickPurchase);
	}
}

void UPurchaseRow::InitializeRow(TObjectPtr<UPurchaseCategory> PurchaseCategory)
{
	_purchaseCategory = PurchaseCategory;
}

void UPurchaseRow::OnClickPurchase()
{
	_currentPurchaseData->OnPurchaseRequested.ExecuteIfBound();
	if (_purchaseCategory)
	{
		_purchaseCategory->OnSelectCategory();
	}
}

void UPurchaseRow::UpdateRow(FPurchaseData* PurchaseData)
{
	_currentPurchaseData = PurchaseData;
	const FPurchaseData& _purchaseData = *_currentPurchaseData;

	IMG_Icon->SetBrushFromTexture(_purchaseData.Image);
	TXT_Description->SetText(FText::FromString(_purchaseData.Description));

	// Current value
	const float _currentValue = _purchaseData.Value.MaxValue;
	TXT_CurrentValue->SetText(FText::FromString(ParseFloat2Text(_currentValue)));

	// Next value
	float _nextValue = _currentValue;
	if (_purchaseData.IsPurchaseable)
	{
		_nextValue = UShopManager::CalculateValue(_purchaseData.InitValue, _purchaseData.IncreasePerValue, _purchaseData.Level.CurrentValue + 1);
	}
	TXT_NextValue->SetText(FText::FromString(ParseFloat2Text(_nextValue)));

	TXT_PurchaseDollar->SetText(FText::FromString(ParseFloat2Text(_purchaseData.PurchaseDollar)));

	// Purchase block
	BTN_Purchase->SetClickable(_purchaseData.IsPurchaseable);
}

FString UPurchaseRow::ParseFloat2Text(float Value)
{
	// 허용 오차 (부동소수점 비교용)
	const float _tolerance = KINDA_SMALL_NUMBER;

	// 1. 자연수 판별
	if (FMath::IsNearlyEqual(Value, FMath::RoundToFloat(Value), _tolerance))
	{
		const int32 _intValue = FMath::RoundToInt(Value);
		return FString::Printf(TEXT("%d"), _intValue);
	}

	// 2. 소수점 한 자리까지만 의미 있는지 판별
	const float _oneDecimal = FMath::RoundToFloat(Value * 10.f) / 10.f;

	if (FMath::IsNearlyEqual(Value, _oneDecimal, _tolerance))
	{
		return FString::Printf(TEXT("%.1f"), _oneDecimal);
	}

	// 3. 그 외 → 소수점 두 자리
	const float _twoDecimal = FMath::RoundToFloat(Value * 100.f) / 100.f;
	return FString::Printf(TEXT("%.2f"), _twoDecimal);
}