// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PurchaseCategory.generated.h"

class UUIPanelShop;
class UInteractableButton;
class UImage;
class UTextBlock;

UCLASS()
class UPurchaseCategory : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Select = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInteractableButton> BTN_Category = nullptr;

private:
	TObjectPtr<UUIPanelShop> _uiPanelShop = nullptr;

	E_PURCHASE_CATEGORY _category = E_PURCHASE_CATEGORY::END;

	UPROPERTY(VisibleAnywhere, Category = "PurchaseCategory|Color")
	FLinearColor _selectedColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, Category = "PurchaseCategory|Color")
	FLinearColor _unselectedColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);

public:
	void InitializeCategory(TObjectPtr<UUIPanelShop> UIShop, E_PURCHASE_CATEGORY Category);

	UFUNCTION()
	void ChangeSelect(bool IsSelected);

	UFUNCTION()
	void OnSelectCategory();
	
};
