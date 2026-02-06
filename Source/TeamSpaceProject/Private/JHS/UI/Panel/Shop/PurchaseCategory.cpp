// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Shop/PurchaseCategory.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "JHS/GameControl/CommonEnums.h"

void UPurchaseCategory::InitializeCategory(E_PURCHASE_CATEGORY Category)
{
	_category = Category;

	FString _categoryName = "Unknown";
	if (TXT_Category != nullptr)
	{
		switch (Category)
		{
			case E_PURCHASE_CATEGORY::SpaceShip:
				_categoryName = TEXT("우주선");
				break;

			case E_PURCHASE_CATEGORY::CollectTool:
				_categoryName = TEXT("회수 장비");
				break;

			case E_PURCHASE_CATEGORY::Turret:
				_categoryName = TEXT("포탑");
				break;

			case E_PURCHASE_CATEGORY::Ammo:
				_categoryName = TEXT("탄약");
				break;
		}
	}

	TXT_Category->SetText(FText::FromString(_categoryName));

	ChangeSelect(false);
}

void UPurchaseCategory::ChangeSelect(bool IsSelected)
{
	if (IMG_Select == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UPurchaseCategory: IMG_Select is nullptr"));
		return;
	}

	FLinearColor _color = IsSelected ? _selectedColor : _unselectedColor;
	IMG_Select->SetColorAndOpacity(_color);
}
