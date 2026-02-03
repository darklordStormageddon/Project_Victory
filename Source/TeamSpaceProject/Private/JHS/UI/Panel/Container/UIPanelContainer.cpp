// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/Container/UIPanelContainer.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/UI/Panel/Container/ContainerItemSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UUIPanelContainer::NativeOnInitialized()
{
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_containerStateGroup = _outGameState->GetContainerStateGroup();
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
	if (Event == nullptr)
		return;

	const FElementData _elementData = Event->ElementData;
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
	
	SortItemSlot();
}

TObjectPtr<UContainerItemSlot> UUIPanelContainer::CreateAndRegisterElementSlot(E_ELEMENT_TYPE ElementType)
{
	if (SB_Items == nullptr || _itemSlotClass == nullptr)
		return nullptr;

	APlayerController* _playerController = GetOwningPlayer();
	if (_playerController == nullptr)
		return nullptr;

	UContainerItemSlot* _slotWidget = CreateWidget<UContainerItemSlot>(_playerController, _itemSlotClass);
	if (_slotWidget == nullptr)
		return nullptr;

	//_slotWidget->InitializeSlot(_slotSizeWidth, _slotSizeHeight);
	SB_Items->AddChild(_slotWidget);

	_elementSlotMap.Add(ElementType, _slotWidget);
	_currentSlotCount++;

	return _slotWidget;
}

void UUIPanelContainer::SortItemSlot()
{
	if (SB_Items == nullptr)
		return;

	TArray<int32> _elementIndexArray;
	for (auto& _element : _elementSlotMap)
	{
		_elementIndexArray.Add(int32(_element.Key));
	}

	_elementIndexArray.Sort();

	// ScrollBox의 모든 자식을 제거하고 정렬된 순서로 다시 추가
	SB_Items->ClearChildren();

	for (int32 i = 0; i < _elementIndexArray.Num(); ++i)
	{
		TObjectPtr<UContainerItemSlot> _slot = _elementSlotMap.FindRef((E_ELEMENT_TYPE)_elementIndexArray[i]);
		if (_slot != nullptr)
		{
			SB_Items->AddChild(_slot);
		}
	}

	int32 _cumulativePrice = _containerStateGroup->GetCumulativePrice();
	TXT_CumulativePrice->SetText(FText::AsNumber(_cumulativePrice));

	int32 _ownedDollar = _containerStateGroup->GetOwnedDollar();
	TXT_OwnedDollar->SetText(FText::AsNumber(_ownedDollar));

	TXT_ExpectDollar->SetText(FText::AsNumber(_cumulativePrice + _ownedDollar));
}

