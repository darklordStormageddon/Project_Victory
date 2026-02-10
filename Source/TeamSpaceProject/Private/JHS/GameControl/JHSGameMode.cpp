// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/SpaceManager.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"

AJHSGameMode::AJHSGameMode()
{
	_uiManager = CreateDefaultSubobject<UUIManager>(TEXT("UIManager"));
	_eventManager = CreateDefaultSubobject<UEventManager>(TEXT("EventManager"));
	_spaceManager = CreateDefaultSubobject<USpaceManager>(TEXT("SpaceManager"));
}

void AJHSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AJHSGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AJHSGameMode::StartGame(AActor* Caller)
{
	if (!CheckIsServerCaller(Caller))
		return;

	// 이미 게임이 시작되었으면 중복 실행 방지
	if (_isGameStarted)
		return;

	_isGameStarted = true;
	_currentStage = 0;
	StartNextStage(Caller);
}

void AJHSGameMode::StartNextStage(AActor* Caller)
{
	if (!CheckIsServerCaller(Caller))
		return;

	// 이미 스테이지가 시작되었으면 중복 실행 방지
	if (_isStageStarted)
		return;

	_isStageStarted = true;
	_currentStage++;

	if (_currentStage == 1)
	{
		AJHSGameState* _outGameState = nullptr;
		if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
			return;

		TObjectPtr<UTurretStateGroup> _turretStateGroup = _outGameState->GetTurretStateGroup();
		_turretStateGroup->SetInfiniteMagMode(true);
		for (auto& _startEquipTurret : _startEquipTurretArray)
		{
			_turretStateGroup->TryEquipTurret(_startEquipTurret.TurretPosition, _startEquipTurret.AmmoType);
		}
	}

	UEventOnStartStage* _event = NewObject<UEventOnStartStage>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: Failed to create UEventOnStartStage"));
		return;
	}

	_event->State = _currentStage;
	_eventManager->ExecuteEvent<UEventOnStartStage>(_event);
}

void AJHSGameMode::EndStage(AActor* Caller)
{
	if (!CheckIsServerCaller(Caller))
		return;

	if (!_isStageStarted)
		return;

	_isStageStarted = false;

	UEventOnEndStage* _event = NewObject<UEventOnEndStage>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: Failed to create UEventOnEndStage"));
		return;
	}

	_eventManager->ExecuteEvent<UEventOnEndStage>(_event);
}

bool AJHSGameMode::CheckIsServerCaller(AActor* Caller)
{
	// 서버가 아니면 실행하지 않음
	if (Caller == nullptr || !Caller->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameMode::Not called from server"));
		return false;
	}

	return true;
}