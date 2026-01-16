// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Container/UIPanelContainer.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/UI/Panel/Container/ContainerItemSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

bool UUIPanelContainer::Initialize()
{
	if (!Super::Initialize())
		return false;

	UG_Items->SetMinDesiredSlotWidth(_slotSizeWidth);
	UG_Items->SetMinDesiredSlotHeight(_slotSizeHeight);

	return true;
}

void UUIPanelContainer::RegisterEvent()
{
	_eventHandleOnChangeElementData = GetEventManager()->AddListener<UEventOnChangeElementData>(
		[this](UEventOnChangeElementData* Event)
		{
			OnChangeElementData(Event);
		}
	);
}

void UUIPanelContainer::UnregisterEvent()
{
	if (_eventHandleOnChangeElementData.IsValid())
	{
		GetEventManager()->DelListener<UEventOnChangeElementData>(_eventHandleOnChangeElementData);
		_eventHandleOnChangeElementData.Reset();
	}
}

void UUIPanelContainer::OnOpen()
{
	_currentSlotCount = _elementSlotMap.Num();
}

void UUIPanelContainer::OnClose()
{

}

void UUIPanelContainer::OnChangeElementData(UEventOnChangeElementData* Event)
{
	if (Event == nullptr || UG_Items == nullptr)
		return;

	const E_ELEMENT_TYPE _elementType = Event->ElementType;
	const bool _isRemove = Event->Amount <= 0;

	if (_isRemove)
	{
		TObjectPtr<UContainerItemSlot> _removeSlot = nullptr;
		if (_elementSlotMap.RemoveAndCopyValue(_elementType, _removeSlot))
		{
			if (_removeSlot != nullptr)
			{
				UG_Items->RemoveChild(_removeSlot);
			}

			if (_currentSlotCount > 0)
			{
				_currentSlotCount--;
			}
		}
	}
	else
	{
		UContainerItemSlot* _slotWidget = nullptr;
		if (!_elementSlotMap.Contains(_elementType))
		{
			_slotWidget = CreateAndRegisterElementSlot(_elementType);
			if (_slotWidget == nullptr)
				return;
		}
		else
		{
			_slotWidget = _elementSlotMap.FindRef(_elementType);
			if (_slotWidget == nullptr)
				return;
		}

		const FString _elementName = CommonEnums::GetEnum2FString<E_ELEMENT_TYPE>(_elementType);

		_slotWidget->UpdateItemInfo(_elementName, Event->Amount);
	}
	
	SortItemSlot();
}

void UUIPanelContainer::SetSlotIndex(TObjectPtr<UContainerItemSlot> ItemSlot, int32 Index)
{
    if (ItemSlot == nullptr)
        return;

    const int32 _columnCount = 3;
    const int32 _row = Index / _columnCount;
    const int32 _column = Index % _columnCount;

	ItemSlot->GetSlot()->SetRow(_row);
	ItemSlot->GetSlot()->SetColumn(_column);
}

TObjectPtr<UContainerItemSlot> UUIPanelContainer::CreateAndRegisterElementSlot(E_ELEMENT_TYPE ElementType)
{
	if (UG_Items == nullptr || _itemSlotClass == nullptr)
		return nullptr;

	APlayerController* _playerController = GetOwningPlayer();
	if (_playerController == nullptr)
		return nullptr;

	UContainerItemSlot* _slotWidget = CreateWidget<UContainerItemSlot>(_playerController, _itemSlotClass);
	if (_slotWidget == nullptr)
		return nullptr;

	UUniformGridSlot* _gridSlot = UG_Items->AddChildToUniformGrid(_slotWidget);
	_slotWidget->InitializeSlot(_slotSizeWidth, _slotSizeHeight);
	if (_gridSlot == nullptr)
		return nullptr;

    SetSlotIndex(_slotWidget, _elementSlotMap.Num());
	_gridSlot->SetHorizontalAlignment(HAlign_Fill);
	_gridSlot->SetVerticalAlignment(VAlign_Fill);

	_elementSlotMap.Add(ElementType, _slotWidget);
	_currentSlotCount++;

	return _slotWidget;
}

void UUIPanelContainer::SortItemSlot()
{
    TArray<int32> _elementIndexArray;
    for (auto& _element : _elementSlotMap)
    {
        _elementIndexArray.Add(int32(_element.Key));
    }

    _elementIndexArray.Sort();

    for (int32 i = 0; i < _elementIndexArray.Num(); ++i)
    {
        SetSlotIndex(_elementSlotMap.FindRef((E_ELEMENT_TYPE)_elementIndexArray[i]), i);
    }
}

