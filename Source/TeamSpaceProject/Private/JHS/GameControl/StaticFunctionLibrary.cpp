// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/Event/EventManager.h"
#include "Engine/Engine.h"

TWeakObjectPtr<AJHSPlayerController> UStaticFunctionLibrary::_cachedLocalPlayerController = nullptr;

bool UStaticFunctionLibrary::TryGetGameMode(AActor* Caller, AJHSGameMode*& OutGameMode)
{
	if (Caller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetGameMode: Caller is nullptr"));
		return false;
	}

	if (!Caller->HasAuthority())
		return false;

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

bool UStaticFunctionLibrary::TryGetPlayerController(AJHSPlayerController*& OutPlayerController)
{
	UWorld* _world = nullptr;
	if (!TryGetWorld(_world))
		return false;

	APlayerController* _playerController = UGameplayStatics::GetPlayerController(_world, 0);
	if (_playerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetPlayerController:: PlayerController is nullptr"));
		return false;
	}

	OutPlayerController = Cast<AJHSPlayerController>(_playerController);
	if (OutPlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetPlayerController:: PlayerController is not AJHSPlayerController"));
		return false;
	}

	return true;
}

int32 UStaticFunctionLibrary::GetAssignedPlayerId()
{
	AJHSPlayerController* _localController = nullptr;
	if (!GetOrCacheLocalPlayerController(_localController))
		return -1;

	return _localController->GetAssignedPlayerId();
}

bool UStaticFunctionLibrary::CheckIsSelfClient(int32 CallerAssignedPlayerId)
{
	AJHSPlayerController* _localController = nullptr;
	if (!GetOrCacheLocalPlayerController(_localController))
		return false;

	return _localController->GetAssignedPlayerId() == CallerAssignedPlayerId;
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

bool UStaticFunctionLibrary::TryGetSpaceManager(AActor* Caller, USpaceManager*& OutSpaceManager)
{
	AJHSGameMode* _gameMode = nullptr;
	if (!TryGetGameMode(Caller, _gameMode))
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
	AJHSPlayerController* _jhsPlayerController = nullptr;
	if (!TryGetPlayerController(_jhsPlayerController))
		return false;

	OutUIManager = _jhsPlayerController->GetUIManager();
	if (OutUIManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TryGetUIManager: UIManager is nullptr"));
		return false;
	}

	return true;
}

bool UStaticFunctionLibrary::TryGetEventManager(UEventManager*& OutEventManager)
{
	// GameState는 서버·클라이언트 모두 존재하므로 EventManager를 여기서 제공 (GameMode는 클라이언트에 없음)
	AJHSGameState* _gameState = nullptr;
	if (!TryGetGameState(_gameState))
	{
		return false;
	}

	OutEventManager = _gameState->GetEventManager();
	if (OutEventManager == nullptr)
	{
		return false;
	}

	return true;
}

float UStaticFunctionLibrary::GetDeltaTime()
{
	UWorld* _world = nullptr;
	if (!TryGetWorld(_world))
		return 0.0f;

	return UGameplayStatics::GetWorldDeltaSeconds(_world);
}

bool UStaticFunctionLibrary::GetOrCacheLocalPlayerController(AJHSPlayerController*& OutController)
{
	UWorld* _world = nullptr;
	if (!TryGetWorld(_world))
		return false;

	if (_world != nullptr && _cachedLocalPlayerController.IsValid() && _cachedLocalPlayerController->IsLocalPlayerController()
		&& _cachedLocalPlayerController->GetWorld() == _world)
	{
		OutController = _cachedLocalPlayerController.Get();
		return true;
	}
	_cachedLocalPlayerController.Reset();

	APlayerController* _pc = nullptr;
	for (FConstPlayerControllerIterator _it = _world->GetPlayerControllerIterator(); _it; ++_it)
	{
		APlayerController* _candidate = _it->Get();
		if (_candidate != nullptr && _candidate->IsLocalPlayerController())
		{
			_pc = _candidate;
			break;
		}
	}
	if (_pc == nullptr)
		return false;

	AJHSPlayerController* _controller = Cast<AJHSPlayerController>(_pc);
	if (_controller == nullptr)
		return false;

	_cachedLocalPlayerController = _controller;
	OutController = _controller;
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