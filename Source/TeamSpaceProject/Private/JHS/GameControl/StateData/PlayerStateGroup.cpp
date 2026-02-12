// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

// Sets default values for this component's properties
UPlayerStateGroup::UPlayerStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlayerStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerStateGroup::InitializePlayerState(TObjectPtr<AJHSGameState> GameState, FPurchaseData PlayerRadiation)
{
	_gameState = GameState;
	_playerRadiationData = PlayerRadiation;
}

void UPlayerStateGroup::UpdatePlayerState()
{
	for (auto& _playerState : _playerStateMap)
	{
		IncreasePlayerRadiation(_playerState.Key, 0.0f);
	}
}

bool UPlayerStateGroup::TryRegistPlayer(TObjectPtr<AJHSPlayerState> PlayerState, FString PlayerName, E_REGIST_ERROR_TYPE& ErrorType)
{
	ErrorType = E_REGIST_ERROR_TYPE::NONE;

	if (PlayerState == nullptr)
	{
		ErrorType = E_REGIST_ERROR_TYPE::NullPlayerState;
		return false;
	}

	int32 _playerID = PlayerState->GetPlayerId();
	if (_playerStateMap.Contains(_playerID))
	{
		ErrorType = E_REGIST_ERROR_TYPE::DuplicatedUID;
		return false;
	}

	for (auto& _playerStateData : _playerStateMap)
	{
		if (_playerStateData.Value.PlayerName.Equals(PlayerName))
		{
			ErrorType = E_REGIST_ERROR_TYPE::DuplicatedName;
			return false;
		}
	}

	FPlayerStateData _newPlayerData;
	_newPlayerData.PlayerState = PlayerState;
	_newPlayerData.PlayerUID = _playerID;
	_newPlayerData.PlayerName = PlayerName;

	_newPlayerData.PlayerIndex = _playerStateMap.Num();
	_newPlayerData.IsReady = false;

	_newPlayerData.Radiation.MaxValue = _playerRadiationData.Value.MaxValue;
	_newPlayerData.Radiation.CurrentValue = _newPlayerData.Radiation.MaxValue;
	_playerStateMap.Add(_newPlayerData.PlayerUID, _newPlayerData);
	return true;
}

void UPlayerStateGroup::ReadyPlayer(int32 PlayerUID)
{
	FPlayerStateData* _outPlayerStateData = nullptr;
	if (!TryGetPlayerStateData(PlayerUID, _outPlayerStateData))
		return;

	_outPlayerStateData->IsReady = true;
}

bool UPlayerStateGroup::IsAllPlayerReady()
{
	for (auto& _element : _playerStateMap)
	{
		FPlayerStateData _playerStateData = _element.Value;
		if (_playerStateData.PlayerState == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerID [%d] is registered but PlayerState is nullptr"), _playerStateData.PlayerUID);
			continue;
		}

		if (!_playerStateData.IsReady)
			return false;
	}

	return true;
}

void UPlayerStateGroup::IncreasePlayerRadiation(int32 PlayerIdx, float IncreaseValue)
{
	FPlayerStateData* _playerState = _playerStateMap.Find(PlayerIdx);
	_playerState->Radiation.CurrentValue += IncreaseValue;
	if (_playerState->Radiation.CurrentValue > _playerState->Radiation.MaxValue)
	{
		_playerState->Radiation.CurrentValue = _playerState->Radiation.MaxValue;
	}

	UEventOnChangePlayerRadiation* _event = NewObject<UEventOnChangePlayerRadiation>(this);
	if (_event == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UPlayerStateGroup: Failed to create UEventOnChangePlayerRadiation"));
		return;
	}

	_event->PlayerStateData = *_playerState;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangePlayerRadiation>(_event);
}

bool UPlayerStateGroup::TryGetPlayerStateData(int32 PlayerUID, FPlayerStateData*& OutPlayerStateData)
{
	if (!_playerStateMap.Contains(PlayerUID))
	{
		UE_LOG(LogTemp, Error, TEXT("UPlayerStateGroup: Invalid PlayerUID: [%d]"), PlayerUID);
		return false;
	}

	OutPlayerStateData = _playerStateMap.Find(PlayerUID);
	return OutPlayerStateData != nullptr;
}