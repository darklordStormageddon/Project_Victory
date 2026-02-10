// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Panel/Container/PlateContainer.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/UI/Panel/Container/ContainerItemSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UPlateContainer::NativeOnInitialized()
{
}

void UPlateContainer::RegisterEvent()
{
	_eventHandleOnChangeElementData = GetEventManager()->AddListener<UEventOnChangeElementData>(
		[this](UEventOnChangeElementData* Event)
		{
			OnChangeElementData(Event);
		}
	);
}

void UPlateContainer::UnregisterEvent()
{
	if (_eventHandleOnChangeElementData.IsValid())
	{
		GetEventManager()->DelListener<UEventOnChangeElementData>(_eventHandleOnChangeElementData);
		_eventHandleOnChangeElementData.Reset();
	}
}

void UPlateContainer::OnOpen()
{
	_currentSlotCount = _elementSlotMap.Num();
}

void UPlateContainer::OnClose()
{
}

void UPlateContainer::OnChangeElementData(UEventOnChangeElementData* Event)
{
	if (Event == nullptr)
		return;

	const FElementData _elementData = Event->ElementData;
	const int32 _cumulativePrice = Event->CumulativePrice;
	const int32 _ownedDollar = Event->OwnedDollar;
	const int32 _goalDollar = Event->GoalDollar;
	const bool _isRemove = _elementData.Amount <= 0;

	if (_isRemove)
	{
		TObjectPtr<UContainerItemSlot> _removeSlot = nullptr;
		if (_elementSlotMap.RemoveAndCopyValue(_elementData.ElementType, _removeSlot))
		{
			if (_removeSlot != nullptr)
			{
				SB_Items->RemoveChild(_removeSlot);
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
		if (!_elementSlotMap.Contains(_elementData.ElementType))
		{
			_slotWidget = CreateAndRegisterElementSlot(_elementData.ElementType);
		}
		else
		{
			_slotWidget = _elementSlotMap.FindRef(_elementData.ElementType);
		}

		if (_slotWidget == nullptr)
			return;

		_slotWidget->UpdateItemInfo(_elementData);
	}

	TXT_CumulativePrice->SetText(FText::AsNumber(_cumulativePrice));
	TXT_OwnedDollar->SetText(FText::AsNumber(_ownedDollar));

	const int32 _expectDollar = _cumulativePrice + _ownedDollar;
	TXT_ExpectDollar->SetText(FText::AsNumber(_expectDollar));
	FLinearColor _expectDollarColor = _expectDollar < _goalDollar ? _lessExpectDollarColor : _overExpectDollarColor;
	TXT_ExpectDollar->SetColorAndOpacity(_expectDollarColor);

	TXT_GoalDollar->SetText(FText::AsNumber(_goalDollar));

	SortItemSlot();
}

TObjectPtr<UContainerItemSlot> UPlateContainer::CreateAndRegisterElementSlot(E_ELEMENT_TYPE ElementType)
{
	if (SB_Items == nullptr || _itemSlotClass == nullptr)
		return nullptr;

	APlayerController* _playerController = GetOwningPlayer();
	if (_playerController == nullptr)
		return nullptr;

	UContainerItemSlot* _slotWidget = CreateWidget<UContainerItemSlot>(_playerController, _itemSlotClass);
	if (_slotWidget == nullptr)
		return nullptr;

	SB_Items->AddChild(_slotWidget);
	_elementSlotMap.Add(ElementType, _slotWidget);
	_currentSlotCount++;

	return _slotWidget;
}

void UPlateContainer::SortItemSlot()
{
	if (SB_Items == nullptr)
		return;

	TArray<int32> _elementIndexArray;
	for (auto& _element : _elementSlotMap)
	{
		_elementIndexArray.Add(int32(_element.Key));
	}

	_elementIndexArray.Sort();
	SB_Items->ClearChildren();

	for (int32 i = 0; i < _elementIndexArray.Num(); ++i)
	{
		TObjectPtr<UContainerItemSlot> _slot = _elementSlotMap.FindRef((E_ELEMENT_TYPE)_elementIndexArray[i]);
		if (_slot != nullptr)
		{
			SB_Items->AddChild(_slot);
		}
	}
}
