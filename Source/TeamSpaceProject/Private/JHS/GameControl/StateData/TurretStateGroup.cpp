// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/Contant/ConstantLibrary.h"
#include "YSH/resource/TurretDataTable.h"
#include "JHS/GameControl/CommonEnums.h"

// Sets default values for this component's properties
UTurretStateGroup::UTurretStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTurretStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTurretStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTurretStateGroup::InitializeTurretState(TObjectPtr<AJHSGameState> GameState, TArray<FAmmoData> AmmoDataArray)
{
	_gameState = GameState;

	_ammoDataMap.Empty();
	for (FAmmoData _ammoData : AmmoDataArray)
	{
		_ammoDataMap.Add(_ammoData.AmmoType, _ammoData);
	}

	LoadTurretDataTable();
}

void UTurretStateGroup::UpdateTurretState()
{
	UEventOnChangeTurretData* _event = NewObject<UEventOnChangeTurretData>(this);
	for (auto& _turretData : _equipTurretMap)
	{
		_event->TurretData = _turretData.Value;
		_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretData>(_event);
	}
}

bool UTurretStateGroup::TryEquipTurret(E_TURRET_POSITION TurretPosition, E_AMMO_TYPE AmmoType)
{
	FTurretData* _outEquipedTurretData = nullptr;
	bool _isExistSamePosition = TryGetEquipedTurret(TurretPosition, _outEquipedTurretData);

	if (_isExistSamePosition && _outEquipedTurretData->AmmoType == AmmoType)
	{
		UE_LOG(LogTemp, Error, TEXT("Same turret position and ammo type"));
		return false;
	}

	FTurretData* _outTurretData = nullptr;
	bool _isMainTurret = TurretPosition == E_TURRET_POSITION::Main;
	if (!TryGetTurretData(_isMainTurret, AmmoType, _outTurretData))
	{
		FString _isMainTurretString = _isMainTurret == true ? TEXT("True") : TEXT("False");
		UE_LOG(LogTemp, Error, TEXT("Failed to get turret data. IsMainTurret: %s, AmmoType: %d"), *_isMainTurretString, (int32)AmmoType);
		return false;
	}

	if (_isExistSamePosition)
	{
		_outEquipedTurretData = _outTurretData;
	}
	else
	{
		FTurretData& _addedTurretData = _equipTurretMap.Add(TurretPosition, *_outTurretData);
		_outEquipedTurretData = &_addedTurretData;
	}
	_outEquipedTurretData->TurretPosition = TurretPosition;

	// 추가되거나 바뀐 데이터를 이벤트로 전송
	UEventOnChangeTurretData* _event = NewObject<UEventOnChangeTurretData>(this);
	_event->TurretData = *_outTurretData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretData>(_event);
	return true;
}

bool UTurretStateGroup::TryGetTurretFireInterval(E_TURRET_POSITION TurretPosition, float* OutFireCoolTime)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretData))
	{
		UE_LOG(LogTemp, Error, TEXT("Not equiped turret. TurretPosition: %d"), (int32)TurretPosition);
		return false;
	}

	*OutFireCoolTime = _outTurretData->FireInterval;
	return true;
}

bool UTurretStateGroup::TryFireTurret(E_TURRET_POSITION TurretPosition)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretData))
	{
		UE_LOG(LogTemp, Error, TEXT("Not equiped turret. TurretPosition: %d"), (int32)TurretPosition);
		return false;
	}

	if (_outTurretData->Mag.CurrentValue <= 0)
		return false;

	ChangeTurretAmmo(_outTurretData, CONSUME_AMMO);
	return true;
}

bool UTurretStateGroup::TryReloadTurret(E_TURRET_POSITION TurretPosition)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetEquipedTurret(TurretPosition, _outTurretData))
	{
		UE_LOG(LogTemp, Error, TEXT("Not equiped turret. TurretPosition: %d"), (int32)TurretPosition);
		return false;
	}

	FAmmoData* _ammoData = _ammoDataMap.Find(_outTurretData->AmmoType);
	if (_ammoData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Not found ammo data. AmmoType: %d"), (int32)_outTurretData->AmmoType);
		return false;
	}

	ChangeTurretAmmo(_outTurretData, _ammoData->ReloadCapacity);
	return true;
}

void UTurretStateGroup::LoadTurretDataTable()
{
	_turretDataMap.Empty();

	TObjectPtr<UDataTable> _turretDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH));
	if (!_turretDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Room Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	TArray<FName> _rowNames = _turretDataTable->GetRowNames();
	for (const FName& RowName : _rowNames)
	{
		FTurretInitState* _turrerInfo = _turretDataTable->FindRow<FTurretInitState>(RowName, TEXT(""));
		if (_turrerInfo)
		{
			E_AMMO_TYPE _outAmmoType = E_AMMO_TYPE::NONE;
			if (!CommonEnums::TryGetAmmoType(_turrerInfo->AmmoType, _outAmmoType))
				return;

			FTurretData _newTurretData;
			_newTurretData.AmmoType = _outAmmoType;
			_newTurretData.Mag.MaxValue = _turrerInfo->InitMaxMag;
			_newTurretData.Mag.CurrentValue = _newTurretData.Mag.MaxValue;
			_newTurretData.FireInterval = _turrerInfo->InitFireInterval;

			_turretDataMap.Add(GetTurretKey(_turrerInfo->bIsMainTurret, _outAmmoType), _newTurretData);
		}
	}

	// 데이터 테이블이 비어있으면 실패
	if (_turretDataMap.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Turret Data Table is empty"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Turret Data Table loaded successfully. %d rows loaded"), _turretDataMap.Num());
	return;
}

int32 UTurretStateGroup::GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType)
{
	return ((int32)IsMainTurret + 1) * HUNDRED + (int32)AmmoType;
}

bool UTurretStateGroup::TryGetEquipedTurret(E_TURRET_POSITION TurretPosition, FTurretData*& OutTurretData)
{
	if (!_equipTurretMap.Contains(TurretPosition))
		return false;

	OutTurretData = _equipTurretMap.Find(TurretPosition);
	return OutTurretData != nullptr;
}

bool UTurretStateGroup::TryGetTurretData(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData)
{
	int32 _turretKey = GetTurretKey(IsMainTurret, AmmoType);
	if (!_turretDataMap.Contains(_turretKey))
		return false;

	OutTurretData = _turretDataMap.Find(_turretKey);
	return OutTurretData != nullptr;
}

void UTurretStateGroup::ChangeTurretAmmo(FTurretData* TurretData, int32 ChangeValue)
{
	TurretData->Mag.CurrentValue += ChangeValue;
	if (TurretData->Mag.CurrentValue > TurretData->Mag.MaxValue)
	{
		TurretData->Mag.CurrentValue = TurretData->Mag.MaxValue;
	}
	if (TurretData->Mag.CurrentValue < 0)
	{
		TurretData->Mag.CurrentValue = 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("%d"), (int32)TurretData->Mag.CurrentValue);
	UEventOnChangeTurretAmmo* _event = NewObject<UEventOnChangeTurretAmmo>(this);
	_event->TurretPosition = TurretData->TurretPosition;
	_event->Ammo = TurretData->Mag;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretAmmo>(_event);
}