// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/UI/Panel/Container/PlateContainer.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/CommonEnums.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/UI/Panel/Container/ContainerItemSlot.h"
#include "JHS/UI/Interact/InteractableScrollBox.h"
#include "Components/TextBlock.h"

void UPlateContainer::NativeOnInitialized()
{
}

void UPlateContainer::NativeConstruct()
{
	UEventManager* _outEventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
		return;

	_eventHandleOnChangeElementData = _outEventManager->AddListener<UEventOnChangeElementData>(
		[this](UEventOnChangeElementData* Event)
		{
			OnChangeElementData(Event);
		}
	);

	_eventHandleOnChangeOwnedDollar = _outEventManager->AddListener<UEventOnChangeOwnedDollar>(
		[this](UEventOnChangeOwnedDollar* Event)
		{
			OnChangeOwnedDollar(Event);
		}
	);

	_currentSlotCount = _elementSlotMap.Num();

	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_outGameState->SendCurrentDataEvent();
}

void UPlateContainer::NativeDestruct()
{
	UEventManager* _outEventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
		return;

	if (_eventHandleOnChangeElementData.IsValid())
	{
		_outEventManager->DelListener<UEventOnChangeElementData>(_eventHandleOnChangeElementData);
		_eventHandleOnChangeElementData.Reset();
	}

	if (_eventHandleOnChangeOwnedDollar.IsValid())
	{
		_outEventManager->DelListener<UEventOnChangeElementData>(_eventHandleOnChangeOwnedDollar);
		_eventHandleOnChangeOwnedDollar.Reset();
	}
}

void UPlateContainer::OnChangeElementData(UEventOnChangeElementData* Event)
{
	if (Event == nullptr)
		return;

	const FElementData _elementData = Event->ElementData;
	const bool _isRemove = _elementData.Amount <= 0;

	// Slot
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

	// Text
	_targetCumulativePrice = Event->CumulativePrice;
	_goalDollar = Event->GoalDollar;
	UpdateDollar();
}

void UPlateContainer::OnChangeOwnedDollar(UEventOnChangeOwnedDollar* Event)
{
	_targetOwnedDollar = Event->OwnedDollar;
	UpdateDollar();
}

void UPlateContainer::UpdateDollar()
{
	const int32 _expectDollar = _targetCumulativePrice + _targetOwnedDollar;
	TXT_ExpectDollar->SetText(FText::AsNumber(_expectDollar));
	FLinearColor _expectDollarColor = _expectDollar < _goalDollar ? _lessExpectDollarColor : _overExpectDollarColor;
	TXT_ExpectDollar->SetColorAndOpacity(_expectDollarColor);

	TXT_GoalDollar->SetText(FText::AsNumber(_goalDollar));

	ClearTimer();
	GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &UPlateContainer::UpdateText, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), false);
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

void UPlateContainer::ClearTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(_timerHandle);
	_currentEffectUpdateTime = 0.0f;
}

void UPlateContainer::UpdateText()
{
	float _deltaTime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());

	bool _isComplete = true;
	// Cumulative Price
	if (!UpdateValue(&_currentCumulativePrice, _targetCumulativePrice, TXT_CumulativePrice))
	{
		_isComplete = false;
	}

	// Owned Dollar
	if (!UpdateValue(&_currentOwnedDollar, _targetOwnedDollar, TXT_OwnedDollar))
	{
		_isComplete = false;
	}
	
	if (_isComplete)
	{
		ClearTimer();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &UPlateContainer::UpdateText, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), false);
}

bool UPlateContainer::UpdateValue(float* CurrentValue, float TargetValue, TObjectPtr<UTextBlock> TextBlock)
{
	if (TextBlock == nullptr)
		return true;

	_currentEffectUpdateTime += UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	if (_currentEffectUpdateTime >= _effectUpdateTime)
	{
		*CurrentValue = TargetValue;
		TextBlock->SetText(FText::AsNumber((int32)*CurrentValue));
		return true;
	}
	
	const float _updateRate = _currentEffectUpdateTime / _effectUpdateTime;
	*CurrentValue = FMath::Lerp(*CurrentValue, TargetValue, _updateRate);
	TextBlock->SetText(FText::AsNumber((int32)*CurrentValue));
	return false;
}