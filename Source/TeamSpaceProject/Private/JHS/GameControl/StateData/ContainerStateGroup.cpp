// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "KSM/DataTable/ElementDataTable.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/CommonEnums.h"

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
	LoadElementData();
}

void UContainerStateGroup::UpdateContainerState()
{
	for (auto& _elementData : _containerState.ElementDataMap)
	{
		ExecuteEventOnChangeElement(_elementData.Value);
	}
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
		return;

	_elementData->Amount += Amount;

	ExecuteEventOnChangeElement(*_elementData);
}

void UContainerStateGroup::SaleAllElement()
{
	SaleElementInternal(0);
}

void UContainerStateGroup::SaleElementInternal(int32 ElementTypeIndex)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("UContainerStateGroup::SaleElementInternal: World is null."));
		return;
	}

	if (ElementTypeIndex >= (int32)E_ELEMENT_TYPE::NONE)
	{
		GetWorld()->GetTimerManager().ClearTimer(_saleAllElementTimerHandle);
		return;
	}

	FElementData* _elementData = nullptr;
	const E_ELEMENT_TYPE _elementType = (E_ELEMENT_TYPE)ElementTypeIndex;
	if (TryGetElementData(_elementType, _elementData))
	{
		int32 _totalDollar = _elementData->Price * _elementData->Amount;
		_containerState.OwnedDollar += _totalDollar;
		_elementData->Amount = 0;

		ExecuteEventOnChangeElement(*_elementData);
	}

	if (_containerState.SaleInterval <= 0.0f)
	{
		_containerState.SaleInterval = 0.5f;
	}

	FTimerDelegate _delegate;
	_delegate.BindUObject(this, &UContainerStateGroup::SaleElementInternal, ElementTypeIndex + 1);
	GetWorld()->GetTimerManager().SetTimer(_saleAllElementTimerHandle, _delegate, _containerState.SaleInterval, false);
}

bool UContainerStateGroup::TryGetElementData(E_ELEMENT_TYPE ElementType, FElementData*& OutElementData)
{
	if (!_containerState.ElementDataMap.Contains(ElementType))
	{
		UE_LOG(LogTemp, Error, TEXT("Not found element data. ElementType: %d"), (int32)ElementType);
		return false;
	}

	OutElementData = _containerState.ElementDataMap.Find(ElementType);
	return OutElementData != nullptr;
}

void UContainerStateGroup::LoadElementData()
{
	// Element
	_containerState.ElementDataMap.Empty();

	TObjectPtr<UDataTable> _elementDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.ELEMENT_PATH));
	if (!_elementDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Element Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.ELEMENT_PATH);
		return;
	}

	TArray<FName> _rowNames = _elementDataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FElementProperty* _elementProperty = _elementDataTable->FindRow<FElementProperty>(_rowName, TEXT(""));
		if (_elementProperty)
		{
			FElementData _newElementData;
			_newElementData.ElementType = _elementProperty->ElementType;
			_newElementData.KRName = _elementProperty->DisplayName;
			_newElementData.Price = _elementProperty->Price;
			_newElementData.Amount = 0;

			// Texture
			FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_ELEMENT_TYPE>(_newElementData.ElementType);
			TObjectPtr<UTexture2D> _outTexture = nullptr;
			if (AJHSGameState::TryGetTextureFromPath(ConstantLibrary::Resource.Image.ELEMENT_FOLDER_PATH, _fileName, _outTexture))
			{
				_newElementData.ElementImage = _outTexture;
			}

			_containerState.ElementDataMap.Add(_newElementData.ElementType, _newElementData);
		}
	}
}

void UContainerStateGroup::ExecuteEventOnChangeElement(FElementData ElementData)
{
	UEventOnChangeElementData* _event = NewObject<UEventOnChangeElementData>(this);
	_event->ElementData = ElementData;

	int32 _cumulativePrice = 0;
	for (auto& _elementData : _containerState.ElementDataMap)
	{
		_cumulativePrice += _elementData.Value.Price * _elementData.Value.Amount;
	}
	_event->CumulativePrice = _cumulativePrice;

	_event->OwnedDollar = _containerState.OwnedDollar;

	_event->GoalDollar = _gameState->GetGoalDollar();

	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeElementData>(_event);
}