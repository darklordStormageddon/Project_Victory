// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

// Sets default values for this component's properties
UContainerStateGroup::UContainerStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UContainerStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UContainerStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UContainerStateGroup::InitializeContainerState(TObjectPtr<AJHSGameState> GameState, FContainerState InitContainerState)
{
	_gameState = GameState;

	_containerState = InitContainerState;
}

void UContainerStateGroup::UpdateContainerState()
{

}

void UContainerStateGroup::AddElement(E_ELEMENT_TYPE ElementType, int32 Amount)
{
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UContainerStateGroup: Amount is less than 0"));
		return;
	}

	FElementData* _elementData = nullptr;
	if (!TryGetElementData(ElementType, _elementData))
	{
		FElementData _newElementData;
		_newElementData.ElementType = ElementType;
		_newElementData.ValueOfElement = 0;
		_newElementData.Amount = Amount;
		_containerState.ElementDataMap.Add(_newElementData.ElementType, _newElementData);

		_elementData = &_newElementData;
	}
	else
	{
		_elementData->Amount += Amount;
	}

	UEventOnChangeElementData* _event = NewObject<UEventOnChangeElementData>(this);
	_event->ElementType = ElementType;
	_event->Amount = _elementData->Amount;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeElementData>(_event);
}

void UContainerStateGroup::RemoveElement(E_ELEMENT_TYPE ElementType, int32 Amount)
{
	FElementData* _elementData = nullptr;
	if (!TryGetElementData(ElementType, _elementData))
		return;

	_elementData->Amount -= Amount;
	int32 _amount = _elementData->Amount;
	if (_elementData->Amount <= 0)
	{
		_containerState.ElementDataMap.Remove(ElementType);
		_amount = 0;
	}

	UEventOnChangeElementData* _event = NewObject<UEventOnChangeElementData>(this);
	_event->ElementType = ElementType;
	_event->Amount = _amount;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeElementData>(_event);
}

bool UContainerStateGroup::TryGetElementData(E_ELEMENT_TYPE ElementType, FElementData*& OutElementData)
{
	if (!_containerState.ElementDataMap.Contains(ElementType))
		return false;

	OutElementData = _containerState.ElementDataMap.Find(ElementType);
	return OutElementData != nullptr;
}