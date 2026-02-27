// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PurchaseRow.generated.h"

class UImage;
class UTextBlock;
class UInteractableButton;
class UPurchaseCategory;

UCLASS()
class UPurchaseRow : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Icon = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Description = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_CurrentValue = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_NextValue = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInteractableButton> BTN_Purchase = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_PurchaseDollar = nullptr;

private:
	TObjectPtr<UPurchaseCategory> _purchaseCategory = nullptr;

	FPurchaseData* _currentPurchaseData;

protected:
	virtual void NativeOnInitialized() override;

public:
	void InitializeRow(TObjectPtr<UPurchaseCategory> PurchaseCategory);

	void UpdateRow(FPurchaseData* PurchaseData);

private:
	UFUNCTION()
	void OnClickPurchase();
};
