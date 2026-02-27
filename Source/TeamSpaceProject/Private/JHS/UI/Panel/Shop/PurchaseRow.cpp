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

	const float _currentValue = _purchaseData.Value.CurrentValue;
	TXT_CurrentValue->SetText(FText::FromString(FString::Printf(TEXT("%0.2f"), _currentValue)));
	const float _nextValue = UShopManager::CalculateValue(_purchaseData.InitValue, _purchaseData.IncreasePerValue, _purchaseData.Level.CurrentValue + 1);
	TXT_NextValue->SetText(FText::FromString(FString::Printf(TEXT("%0.2f"), _nextValue)));

	TXT_PurchaseDollar->SetText(FText::FromString(FString::Printf(TEXT("%d"), (int32)_purchaseData.PurchaseDollar)));

	// Purchase block
	BTN_Purchase->SetClickable(_purchaseData.IsPurchaseable);
}