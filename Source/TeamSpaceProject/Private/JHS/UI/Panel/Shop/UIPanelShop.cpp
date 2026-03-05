// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Shop/UIPanelShop.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/UI/Panel/Shop/PurchaseCategory.h"
#include "JHS/UI/Panel/Shop/PurchaseRow.h"
#include "JHS/UI/Panel/Container/PlateContainer.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "JHS/UI/Interact/InteractableScrollBox.h"
#include "Components/SizeBox.h"
#include "Blueprint/UserWidget.h"
#include "JHS/UI/Interact/InteractableButton.h"
#include "JHS/GameControl/CommonEnums.h"

void UUIPanelShop::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_gameState = _outGameState;

	CreatePurchaseCategories();
	SB_ScrollBox->ClearChildren();
	_purchaseRowMap.Empty();

	if (SB_PlateContainer != nullptr && _plateContainerClass != nullptr)
	{
		APlayerController* _playerController = GetOwningPlayer();
		if (_playerController != nullptr)
		{
			_plateContainer = CreateWidget<UPlateContainer>(_playerController, _plateContainerClass);
			if (_plateContainer != nullptr)
			{
				SB_PlateContainer->AddChild(_plateContainer);
			}
		}
	}

	// BTN_SaleElement
	if (BTN_SaleElement != nullptr)
	{
		BTN_SaleElement->ClickedEnter.AddDynamic(this, &UUIPanelShop::OnClickSaleElement);
		BTN_SaleElement->InitializeButton(TEXT("판매"));
	}
}

void UUIPanelShop::RegisterEvent()
{
	TObjectPtr<UEventManager> EventManager = GetEventManager();
	if (EventManager != nullptr)
	{
		_eventHandleOnPurchase = EventManager->AddListener<UEventOnPurchase>(
			[this](UEventOnPurchase* Event)
			{
				OnChangeSpaceShipData(Event);
			}
		);
	}
}

void UUIPanelShop::UnregisterEvent()
{
	TObjectPtr<UEventManager> EventManager = GetEventManager();
	if (EventManager != nullptr)
	{
		if (_eventHandleOnPurchase.IsValid())
		{
			EventManager->DelListener<UEventOnPurchase>(_eventHandleOnPurchase);
			_eventHandleOnPurchase.Reset();
		}
	}
}

void UUIPanelShop::OnOpen()
{
	Super::OnOpen();

	SelectCategory(E_PURCHASE_CATEGORY(0));

	_elementIndex = 0;
	//GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &UUIPanelShop::AddElement, 3.0f, false);
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
		_categoryWidget->InitializeCategory(this, _category);

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

	UpdatePurchaseRow(_selectedCategory, _categoryButton);
}

void UUIPanelShop::UpdatePurchaseRow(E_PURCHASE_CATEGORY SelectedCategory, TObjectPtr<UPurchaseCategory> CategoryButton)
{
	if (_gameState == nullptr || _gameState->GetCollectStateGroup() == nullptr)
		return;

	if (SB_ScrollBox == nullptr || _purchaseRowClass == nullptr)
		return;

	SB_ScrollBox->ClearChildren();

	TArray<FPurchaseData*> _purchaseDataArray = GetPurchaseDataArray(SelectedCategory);
	int32 _activeCount = _purchaseDataArray.Num();

	TObjectPtr<UPurchaseRow> _rowWidget = nullptr;
	for (int32 i = 0; i < _activeCount; i++)
	{
		if (_purchaseRowMap.Contains(i))
		{
			_rowWidget = _purchaseRowMap[i];
		}
		else
		{
			// _purchaseDataArray 수보다 적으면 Row 추가
			_rowWidget = CreateWidget<UPurchaseRow>(this, _purchaseRowClass);
			if (_rowWidget == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("UUIPanelShop: Failed to create PurchaseRow widget for index %d"), i);
				break;
			}
			_purchaseRowMap.Add(i, _rowWidget);
		}

		_rowWidget->InitializeRow(CategoryButton);
		FPurchaseData* _purchaseData = _purchaseDataArray[i];
		_rowWidget->UpdateRow(_purchaseData);
		_rowWidget->SetVisibility(ESlateVisibility::Visible);
		SB_ScrollBox->AddChild(_rowWidget);
	}

	// _purchaseRowMap.Num()+1 ~ _purchaseRowMap.Num() 비활성화
	for (int32 i = _activeCount; i < _purchaseRowMap.Num(); i++)
	{
		TObjectPtr<UPurchaseRow>* _rowWidgetPtr = _purchaseRowMap.Find(i);
		if (_rowWidgetPtr == nullptr || *_rowWidgetPtr == nullptr)
			continue;
		
		(*_rowWidgetPtr)->SetVisibility(ESlateVisibility::Collapsed);
	}
}

TArray<FPurchaseData*> UUIPanelShop::GetPurchaseDataArray(E_PURCHASE_CATEGORY SelectedCategory)
{
	TArray<FPurchaseData*> _purchaseDataArray;

	switch (SelectedCategory)
	{
	case E_PURCHASE_CATEGORY::SpaceShip:
		_purchaseDataArray = _gameState->GetSpaceShipStateGroup()->GetPurchaseDataArray();
		break;

	case E_PURCHASE_CATEGORY::CollectTool:
		_purchaseDataArray = _gameState->GetCollectStateGroup()->GetPurchaseDataArray();
		break;

	case E_PURCHASE_CATEGORY::Turret:
		_purchaseDataArray = _gameState->GetTurretStateGroup()->GetTurretPurchaseDataArray();
		break;

	case E_PURCHASE_CATEGORY::Ammo:
		_purchaseDataArray = _gameState->GetTurretStateGroup()->GetAmmoPurchaseDataArray();
		break;
	}

	return _purchaseDataArray;
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

void UUIPanelShop::OnChangeSpaceShipData(UEventOnPurchase* Event)
{
	if (Event == nullptr)
		return;

	SelectCategory(_selectedCategory);
}

void UUIPanelShop::OnClickSaleElement()
{
	_gameState->GetContainerStateGroup()->SaleAllElement();
}

void UUIPanelShop::AddElement()
{
	if (_gameState == nullptr)
		return;

	UContainerStateGroup* _containerStateGroup = _gameState->GetContainerStateGroup();
	if (_containerStateGroup == nullptr)
		return;

	_elementIndex++;
	if (_elementIndex > (int32)E_ELEMENT_TYPE::CarbonFiber)
		return;
	
	_containerStateGroup->AddElement((E_ELEMENT_TYPE)_elementIndex, _elementIndex + 1);
	GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &UUIPanelShop::AddElement, 2.0f, false);
}