// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/Event/EventManager.h"
#include "Engine/Engine.h"

bool UStaticFunctionLibrary::TryGetGameMode(AJHSGameMode*& OutGameMode)
{
	UWorld* _world = nullptr;
	if (!TryGetWorld(_world))
		return false;

	AGameModeBase* _gameModeBase = UGameplayStatics::GetGameMode(_world);
	if (!_gameModeBase)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetGameMode: GameModeBase is nullptr"));
		return false;
	}

	OutGameMode = Cast<AJHSGameMode>(_gameModeBase);
    if (OutGameMode == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TryGetGameMode: GameMode is nullptr, GameModeBase class: %s"), *_gameModeBase->GetClass()->GetName());
        return false;
    }

	return true;
}

bool UStaticFunctionLibrary::TryGetGameState(AJHSGameState*& OutGameState)
{
	UWorld* _world = nullptr;
	if (!TryGetWorld(_world))
		return false;

	AGameStateBase* _gameStateBase = UGameplayStatics::GetGameState(_world);
	if (!_gameStateBase)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetGameState: GameStateBase is nullptr"));
		return false;
	}

	OutGameState = Cast<AJHSGameState>(_gameStateBase);
	if (OutGameState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetGameState: GameState is nullptr, GameStateBase class: %s"), *_gameStateBase->GetClass()->GetName());
		return false;
	}

	return true;
}

bool UStaticFunctionLibrary::TryGetSpaceManager(USpaceManager*& OutSpaceManager)
{
	AJHSGameMode* _gameMode = nullptr;
	if (!TryGetGameMode(_gameMode))
        return false;

	OutSpaceManager = _gameMode->GetSpaceManager();
    if (OutSpaceManager == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TryGetSpaceManager: SpaceManager is nullptr"));
        return false;
    }

	return true;
}

bool UStaticFunctionLibrary::TryGetUIManager(UUIManager*& OutUIManager)
{
	AJHSGameMode* _gameMode = nullptr;
	if (!TryGetGameMode(_gameMode))
		return false;

	OutUIManager = _gameMode->GetUIManager();
	if (OutUIManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetUIManager: UIManager is nullptr"));
		return false;
	}

	return true;
}

bool UStaticFunctionLibrary::TryGetEventManager(UEventManager*& OutEventManager)
{
	AJHSGameMode* _gameMode = nullptr;
	if (!TryGetGameMode(_gameMode))
		return false;

	OutEventManager = _gameMode->GetEventManager();
	if (OutEventManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetEventManager: EventManager is nullptr"));
		return false;
	}

	return true;
}

bool UStaticFunctionLibrary::TryGetWorld(UWorld*& OutWorld)
{
	UWorld* _world = nullptr;

	// GEngine을 통해 게임 World 찾기
	if (GEngine)
	{
		// 모든 World Context를 순회하면서 게임 World 찾기
		for (const FWorldContext& _context : GEngine->GetWorldContexts())
		{
			UWorld* _checkWorld = _context.World();
			if (_checkWorld && (_checkWorld->WorldType == EWorldType::Game || _checkWorld->WorldType == EWorldType::PIE))
			{
				_world = _checkWorld;
				break;
			}
		}

		// 게임 World를 찾지 못한 경우, 첫 번째 World 사용
		if (!_world && GEngine->GetWorldContexts().Num() > 0)
		{
			_world = GEngine->GetWorldContexts()[0].World();
		}
	}

	OutWorld = _world;
	if (!OutWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetWorld: World is nullptr"));
		return false;
	}

	return true;
}