// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
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

void UContainerStateGroup::InitializeContainerState(TObjectPtr<AJHSGameState> GameState, FContainerState InitContainerState, TArray<FAmmoData> AmmoDataArray)
{
	_gameState = GameState;

	_containerState = InitContainerState;
	for (auto _ammoData : AmmoDataArray)
	{
		_containerState.AmmoDataMap.Add(_ammoData.AmmoType, _ammoData);
	}
	LoadResource();
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
		_elementData = _containerState.ElementDataMap.Find(ElementType);
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
	{
		UE_LOG(LogTemp, Error, TEXT("Not found element data. ElementType: %d"), (int32)ElementType);
		return false;
	}

	OutElementData = _containerState.ElementDataMap.Find(ElementType);
	return OutElementData != nullptr;
}

bool UContainerStateGroup::TryGetAmmoData(E_AMMO_TYPE AmmoType, FAmmoData*& OutAmmoData)
{
	if (!_containerState.AmmoDataMap.Contains(AmmoType))
	{
		UE_LOG(LogTemp, Error, TEXT("Not found ammo data. AmmoType: %d"), (int32)AmmoType);
		return false;
	}

	OutAmmoData = _containerState.AmmoDataMap.Find(AmmoType);
	return OutAmmoData != nullptr;
}

void UContainerStateGroup::LoadResource()
{
	bool _isReadyElementData = false;
	// Element
	if (_isReadyElementData)
	{
		TMap<E_ELEMENT_TYPE, FElementData> _elementDataMap;

		TObjectPtr<UDataTable> _elementDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.ELEMENT_INFO_PATH));
		if (!_elementDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Room Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.ELEMENT_INFO_PATH);
			return;
		}

		TArray<FName> _rowNames = _elementDataTable->GetRowNames();
		for (const FName& _rowName : _rowNames)
		{
			//FTurretInitState* _turrerInfo = _turretDataTable->FindRow<FTurretInitState>(_rowName, TEXT(""));
			if (_isReadyElementData) //_turrerInfo)
			{
				E_ELEMENT_TYPE _outElementType = E_ELEMENT_TYPE::NONE;
				if (!CommonEnums::TryGetElementType("", _outElementType))
					return;

				FElementData _newElementData;
				_newElementData.ElementType = _outElementType;
				_newElementData.ValueOfElement = 0;
				_newElementData.Amount = 0;
				// Texture
				FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_ELEMENT_TYPE>(_newElementData.ElementType);
				FString _texturePath = ConstantLibrary::Resource.Image.AMMO_FOLDER_PATH + _fileName + "." + _fileName;
				UTexture2D* _loadedTexture = LoadObject<UTexture2D>(nullptr, *_texturePath);
				if (_loadedTexture != nullptr)
				{
					_newElementData.ElementImage = _loadedTexture;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("UContainerStateGroup: Failed to load element texture: %s"), *_texturePath);
				}

				_containerState.ElementDataMap.Add(_newElementData.ElementType, _newElementData);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Turret Data Table loaded successfully. %d rows loaded"), _containerState.ElementDataMap.Num());
	}

	// Ammo
	for (auto& _ammoData : _containerState.AmmoDataMap)
	{
		E_AMMO_TYPE _ammoType = _ammoData.Key;
		FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_AMMO_TYPE>(_ammoType);
		FString _texturePath = ConstantLibrary::Resource.Image.AMMO_FOLDER_PATH + _fileName + "." + _fileName;
		UTexture2D* _loadedTexture = LoadObject<UTexture2D>(nullptr, *_texturePath);
		if (_loadedTexture != nullptr)
		{
			_ammoData.Value.AmmoImage = _loadedTexture;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UContainerStateGroup: Failed to load ammo texture: %s"), *_texturePath);
		}
	}
}