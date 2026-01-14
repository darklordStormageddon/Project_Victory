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

void UPlayerStateGroup::InitializePlayerState(TObjectPtr<AJHSGameState> GameState, TArray<FPlayerStateData> PlayerStateArray, float MaxPlayerRadiation)
{
	_gameState = GameState;

	for (FPlayerStateData _playerState : PlayerStateArray)
	{
		_playerState.Radiation.MaxValue = MaxPlayerRadiation;
		_playerState.Radiation.CurrentValue = 0.0f;

		_playerStateMap.Add(_playerState.PlayerIdx, _playerState);
	}
}

void UPlayerStateGroup::UpdatePlayerState()
{
	if (_playerStateMap.Num() > 0)
	{
		for (auto _playerState : _playerStateMap)
		{
			IncreasePlayerRadiation(_playerState.Key, 0.0f);
		}
	}
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