// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PurchaseCategory.generated.h"

class UImage;
class UButton;
class UTextBlock;

UCLASS()
class UPurchaseCategory : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Select = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_Category = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Category = nullptr;

private:
	E_PURCHASE_CATEGORY _category = E_PURCHASE_CATEGORY::END;

	UPROPERTY(VisibleAnywhere, Category = "PurchaseCategory|Color")
	FLinearColor _selectedColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, Category = "PurchaseCategory|Color")
	FLinearColor _unselectedColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);

public:
	void InitializeCategory(E_PURCHASE_CATEGORY Category);

	void ChangeSelect(bool IsSelected);
};
