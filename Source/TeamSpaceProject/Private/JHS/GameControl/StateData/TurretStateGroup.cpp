// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/Contant/ConstantLibrary.h"

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

	// TODO : 터렛 데이터 테이블 캐싱
	// //LoadTurretDataTable();

	// Test 터렛
	FTurretData _testMainTurretData;
	_testMainTurretData.IsMainTurret = true;
	_testMainTurretData.TurretPosition = E_TURRET_POSITION::Main;
	_testMainTurretData.AmmoType = E_AMMO_TYPE::Bullet;
	_testMainTurretData.Ammo.MaxValue = 20;
	_testMainTurretData.Ammo.CurrentValue = 20;
	_testMainTurretData.FireCoolTime = 1.0f;
	TryEquipTurret(_testMainTurretData);

	FTurretData _testLeftTurretData;
	_testLeftTurretData.IsMainTurret = false;
	_testLeftTurretData.TurretPosition = E_TURRET_POSITION::Left;
	_testLeftTurretData.AmmoType = E_AMMO_TYPE::Bullet;
	_testLeftTurretData.Ammo.MaxValue = 100;
	_testLeftTurretData.Ammo.CurrentValue = 100;
	_testLeftTurretData.FireCoolTime = 0.1f;
	TryEquipTurret(_testLeftTurretData);

	FTurretData _testRightTurretData;
	_testRightTurretData.IsMainTurret = false;
	_testRightTurretData.TurretPosition = E_TURRET_POSITION::Right;
	_testRightTurretData.AmmoType = E_AMMO_TYPE::Cannon;
	_testRightTurretData.Ammo.MaxValue = 10;
	_testRightTurretData.Ammo.CurrentValue = 10;
	_testRightTurretData.FireCoolTime = 5.0f;
	TryEquipTurret(_testRightTurretData);
}

void UTurretStateGroup::UpdateTurretState()
{
	UEventOnChangeTurretData* _event = NewObject<UEventOnChangeTurretData>(this);
	for (auto& _turretData : _turretDataMap)
	{
		_event->TurretData = _turretData.Value;
		_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretData>(_event);
	}
}

bool UTurretStateGroup::TryGetTurretFireCoolTime(E_TURRET_POSITION TurretPosition, float* OutFireCoolTime)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, _outTurretData))
		return false;

	*OutFireCoolTime = _outTurretData->FireCoolTime;
	return true;
}

bool UTurretStateGroup::TryFireTurret(E_TURRET_POSITION TurretPosition)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, _outTurretData))
		return false;

	if (_outTurretData->Ammo.CurrentValue <= 0)
		return false;

	ChangeTurretAmmo(_outTurretData, CONSUME_AMMO);
	return true;
}

bool UTurretStateGroup::TryReloadTurret(E_TURRET_POSITION TurretPosition)
{
	FTurretData* _outTurretData = nullptr;
	if (!TryGetTurretData(TurretPosition, _outTurretData))
		return false;

	FAmmoData* _ammoData = _ammoDataMap.Find(_outTurretData->AmmoType);
	if (_ammoData == nullptr)
		return false;

	ChangeTurretAmmo(_outTurretData, _ammoData->ReloadCapacity);
	return true;
}

void UTurretStateGroup::LoadTurretDataTable()
{
	_turretDataMap.Empty();

	TObjectPtr<UDataTable> _turretDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH));

	// 로드 실패 처리
	if (!_turretDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Room Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	// 데이터 테이블에서 모든 행 가져오기
	TArray<FName> _rowNames = _turretDataTable->GetRowNames();
}

bool UTurretStateGroup::TryEquipTurret(FTurretData TurretData)
{
	if ((TurretData.IsMainTurret && TurretData.TurretPosition != E_TURRET_POSITION::Main)
	|| (!TurretData.IsMainTurret && TurretData.TurretPosition == E_TURRET_POSITION::Main))
		return false;

	FTurretData* _outTurretData = nullptr;
	if (TryGetTurretData(TurretData.TurretPosition, _outTurretData))
	{
		// 같은 포지션이며 같은 AmmoType이면 return false
		if (_outTurretData->AmmoType == TurretData.AmmoType)
			return false;

		// 이미 같은 포지션에 있으면 매개변수 데이터로 교환
		*_outTurretData = TurretData;
	}
	else
	{
		// TurretData.TurretPosition이 없으면 새 터렛 추가
		FTurretData& _addedTurretData = _turretDataMap.Add(TurretData.TurretPosition, TurretData);
		_outTurretData = &_addedTurretData;
	}

	// 추가되거나 바뀐 데이터를 이벤트로 전송
	UEventOnChangeTurretData* _event = NewObject<UEventOnChangeTurretData>(this);
	_event->TurretData = *_outTurretData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretData>(_event);
	return true;
}

bool UTurretStateGroup::TryGetTurretData(E_TURRET_POSITION TurretPosition, FTurretData*& OutTurretData)
{
	if (!_turretDataMap.Contains(TurretPosition))
		return false;

	OutTurretData = _turretDataMap.Find(TurretPosition);
	return OutTurretData != nullptr;
}

void UTurretStateGroup::ChangeTurretAmmo(FTurretData* TurretData, int32 ChangeValue)
{
	TurretData->Ammo.CurrentValue += ChangeValue;
	if (TurretData->Ammo.CurrentValue > TurretData->Ammo.MaxValue)
	{
		TurretData->Ammo.CurrentValue = TurretData->Ammo.MaxValue;
	}
	if (TurretData->Ammo.CurrentValue < 0)
	{
		TurretData->Ammo.CurrentValue = 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("%d"), (int32)TurretData->Ammo.CurrentValue);
	UEventOnChangeTurretAmmo* _event = NewObject<UEventOnChangeTurretAmmo>(this);
	_event->TurretPosition = TurretData->TurretPosition;
	_event->Ammo = TurretData->Ammo;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretAmmo>(_event);
}