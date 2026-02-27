// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/GameControl/JHSPlayerState.h"
#include "GameFramework/PlayerState.h"

AJHSGameMode::AJHSGameMode()
{
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

void AJHSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer == nullptr)
		return;

	AJHSPlayerState* _jhsPS = NewPlayer->GetPlayerState<AJHSPlayerState>();
	if (_jhsPS == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameMode::PostLogin - NewPlayer has no AJHSPlayerState, skip TryRegistPlayer"));
		return;
	}

	AJHSGameState* _gameState = GetGameState<AJHSGameState>();
	if (_gameState == nullptr || _gameState->GetPlayerStateGroup() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameMode::PostLogin - GameState or PlayerStateGroup is null, skip TryRegistPlayer"));
		return;
	}

	FString _playerName = _jhsPS->GetPlayerName();
	if (_playerName.IsEmpty())
		_playerName = TEXT("Player");

	E_REGIST_ERROR_TYPE _errorType = E_REGIST_ERROR_TYPE::NONE;
	if (!_gameState->GetPlayerStateGroup()->TryRegistPlayer(_jhsPS, _playerName, _errorType))
	{
		UE_LOG(LogTemp, Warning, TEXT("AJHSGameMode::PostLogin - TryRegistPlayer failed for %s, ErrorType=%d"), *_playerName, (int32)_errorType);
		return;
	}

	const int32 _assignedId = _jhsPS->GetAssignedPlayerId();
	AJHSPlayerController* _jhsPC = Cast<AJHSPlayerController>(NewPlayer);
	if (_jhsPC != nullptr)
		_jhsPC->SetAssignedPlayerId(_assignedId);

	UE_LOG(LogTemp, Log, TEXT("AJHSGameMode::PostLogin - TryRegistPlayer ok, AssignedPlayerId=%d Name=%s (PC에도 저장)"), _assignedId, *_playerName);
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

	AJHSGameState* _outGameState = nullptr;
	if (!TryGetGameState(_outGameState))
		return;

	// 초기 장착 터렛
	TObjectPtr<UTurretStateGroup> _turretStateGroup = _outGameState->GetTurretStateGroup();
	_turretStateGroup->SetStartSettings(_isInfiniteMagMode, _mainTurretType, _startEquipAutoTurretArray);
}

void AJHSGameMode::EndGame(AActor* Caller)
{
	if (!CheckIsServerCaller(Caller))
		return;

	if (!_isGameStarted)
		return;

	_isGameStarted = false;
}

void AJHSGameMode::StartNextStage(AActor* Caller)
{
	if (!_isGameStarted)
		return;

	if (!CheckIsServerCaller(Caller))
		return;

	// 이미 스테이지가 시작되었으면 중복 실행 방지
	if (_isStageStarted)
		return;

	_isStageStarted = true;
	_currentStage++;

	AJHSGameState* _outGameState = nullptr;
	if (!TryGetGameState(_outGameState))
		return;

	// 플레이어 방사선 초기화
	_outGameState->GetPlayerStateGroup()->UpdatePlayerRadiation();

	// 회수 도구 내구도 초기화
	_outGameState->GetCollectStateGroup()->RepairAllTool();

	UEventOnStartStage* _event = NewObject<UEventOnStartStage>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: Failed to create UEventOnStartStage"));
		return;
	}

	_event->Stage = _currentStage;
	_eventManager->ExecuteEvent<UEventOnStartStage>(_event);
}

void AJHSGameMode::EndStage(AActor* Caller)
{
	if (!_isStageStarted)
		return;

	_isStageStarted = false;

	// Event end stage
	UEventOnEndStage* _eventOnEndStage = NewObject<UEventOnEndStage>(this);
	if (_eventOnEndStage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: Failed to create UEventOnEndStage"));
		return;
	}

	_eventManager->ExecuteEvent<UEventOnEndStage>(_eventOnEndStage);

	// Event to lobby
	UEventOnToLobby* _eventOnToLobby = NewObject<UEventOnToLobby>(this);
	if (_eventOnToLobby == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: Failed to create UEventOnToLobby"));
		return;
	}

	_eventManager->ExecuteEvent<UEventOnToLobby>(_eventOnToLobby);

	// repair shield
	AJHSGameState* _outGameState = nullptr;
	if (!TryGetGameState(_outGameState))
		return;

	_outGameState->GetSpaceShipStateGroup()->RepairSpaceShip();
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

bool AJHSGameMode::TryGetGameState(AJHSGameState*& OutGameState)
{
	if (_cachedGameState == nullptr)
	{
		AJHSGameState* _outGameState = nullptr;
		if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
			return false;

		_cachedGameState = _outGameState;
	}
	
	OutGameState = _cachedGameState;
	return OutGameState != nullptr;
}