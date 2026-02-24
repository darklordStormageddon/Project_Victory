// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/TurretMagReload.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/UI/Panel/Turret/UIPanelTurretMagReload.h"

void ATurretMagReload::BeginPlay()
{
	Super::BeginPlay();

	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretMagReload: Failed to get GameState"));
		return;
	}

	_turretStateGroup = _outGameState->GetTurretStateGroup();
}

void ATurretMagReload::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{
	if (_uiTurretMagReload == nullptr)
	{
		_uiTurretMagReload = Cast<UUIPanelTurretMagReload>(OpenedUI);
		if (_uiTurretMagReload == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ATurretMagReload: Failed to cast UI to UUIPanelTurretMagReload"));
			return;
		}
	}

	_uiTurretMagReload->InitializeMagReload(_turretPosition);
	_turretStateGroup->UpdateTurretState();
}

void ATurretMagReload::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact exit"));
}