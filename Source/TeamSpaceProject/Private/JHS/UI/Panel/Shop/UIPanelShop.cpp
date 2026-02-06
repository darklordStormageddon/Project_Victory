// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Shop/UIPanelShop.h"
#include "JHS/UI/Panel/Shop/PurchaseCategory.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/UserWidget.h"
#include "JHS/GameControl/CommonEnums.h"

void UUIPanelShop::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 최초 1회만 카테고리 위젯 생성
	CreatePurchaseCategories();
}

void UUIPanelShop::OnOpen()
{
	Super::OnOpen();

	// 기본 카테고리 선택 (SpaceShip)
	SelectCategory(E_PURCHASE_CATEGORY::SpaceShip);
}

void UUIPanelShop::CreatePurchaseCategories()
{
	if (HB_Category == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelShop: GRP_PurchaseCategory is nullptr"));
		return;
	}

	if (_purchaseCategoryClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIPanelShop: _purchaseCategoryClass is not set"));
		return;
	}

	// 기존 자식 위젯 제거
	HB_Category->ClearChildren();
	_purchaseCategoryMap.Empty();

	// E_PURCHASE_CATEGORY의 0부터 END-1까지 위젯 생성
	for (int32 i = 0; i < (int32)E_PURCHASE_CATEGORY::END; i++)
	{
		E_PURCHASE_CATEGORY _category = (E_PURCHASE_CATEGORY)i;

		// UPurchaseCategory 위젯 생성
		UPurchaseCategory* _categoryWidget = CreateWidget<UPurchaseCategory>(this, _purchaseCategoryClass);
		if (_categoryWidget == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UUIPanelShop: Failed to create PurchaseCategory widget for category %d"), i);
			continue;
		}
		_categoryWidget->InitializeCategory(_category);

		// HorizontalBox에 추가
		UHorizontalBoxSlot* _slot = HB_Category->AddChildToHorizontalBox(_categoryWidget);
		if (_slot != nullptr)
		{
			// Fill로 설정
			_slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		// 맵에 저장
		_purchaseCategoryMap.Add(_category, _categoryWidget);
	}

	UE_LOG(LogTemp, Warning, TEXT("UUIPanelShop: Created %d purchase category widgets"), _purchaseCategoryMap.Num());
}

void UUIPanelShop::SelectCategory(E_PURCHASE_CATEGORY Category)
{
    // 이전에 선택된 카테고리 해제
    TObjectPtr<UPurchaseCategory> _categoryButton = nullptr;
	if (TryGetPurchaseCategory(_selectedCategory, _categoryButton))
	{
		_categoryButton->ChangeSelect(false);
	}

    // 선택된 카테고리 설정
    _selectedCategory = Category;
    if (!TryGetPurchaseCategory(_selectedCategory, _categoryButton))
        return;

	_categoryButton->ChangeSelect(true);
}

bool UUIPanelShop::TryGetPurchaseCategory(E_PURCHASE_CATEGORY Category, TObjectPtr<UPurchaseCategory>& OutPurchaseCategory)
{
    OutPurchaseCategory = nullptr;
	if (!_purchaseCategoryMap.Contains(Category))
	{
        UE_LOG(LogTemp, Error, TEXT("UUIPanelShop: Failed to get purchase category: %s"), *CommonEnums::GetEnum2FString<E_PURCHASE_CATEGORY>(Category));
	    return false;	
	}

    OutPurchaseCategory = _purchaseCategoryMap[Category];
	return true;
}
