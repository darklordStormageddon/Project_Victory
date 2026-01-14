// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

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

void UTurretStateGroup::InitializeTurretState(TObjectPtr<AJHSGameState> GameState, FTurretData InitTurretData)
{
	_gameState = GameState;

	_turretData.Ammo.MaxValue = InitTurretData.Ammo.MaxValue;
	_turretData.Ammo.CurrentValue = _turretData.Ammo.MaxValue;
	_turretData.FireCoolTime = InitTurretData.FireCoolTime;
	_turretData.ReloadAmmo = InitTurretData.ReloadAmmo;
}

void UTurretStateGroup::UpdateTurretState()
{
	ChangeTurretAmmo(0);
}

bool UTurretStateGroup::TryFireTurret()
{
	if (_turretData.Ammo.CurrentValue <= 0)
		return false;

	ChangeTurretAmmo(CONSUME_AMMO);
	return true;
}

void UTurretStateGroup::ReloadTurret()
{
	ChangeTurretAmmo(_turretData.ReloadAmmo);
}

void UTurretStateGroup::ChangeTurretAmmo(int32 ChangeValue)
{
	FMaxCurrentData* _originalData = &_turretData.Ammo;
	_originalData->CurrentValue += ChangeValue;
	if (_originalData->CurrentValue > _originalData->MaxValue)
	{
		_originalData->CurrentValue = _originalData->MaxValue;
	}
	if (_originalData->CurrentValue < 0)
	{
		_originalData->CurrentValue = 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("%d"), (int32)_originalData->CurrentValue);
	UEventOnChangeTurretAmmo* _event = NewObject<UEventOnChangeTurretAmmo>(this);
	_event->Ammo = *_originalData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTurretAmmo>(_event);
}