// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "Engine/Engine.h"

bool UStaticFunctionLibrary::GetGameMode(AJHSGameMode*& OutGameMode)
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
	
	if (!_world)
	{
		UE_LOG(LogTemp, Error, TEXT("GetGameMode: World is nullptr"));
		return false;
	}

	AGameModeBase* _gameModeBase = UGameplayStatics::GetGameMode(_world);
	if (!_gameModeBase)
	{
		UE_LOG(LogTemp, Error, TEXT("GetGameMode: GameModeBase is nullptr"));
		return false;
	}

	OutGameMode = Cast<AJHSGameMode>(_gameModeBase);
    if (OutGameMode == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("GetGameMode: GameMode is nullptr, GameModeBase class: %s"), *_gameModeBase->GetClass()->GetName());
        return false;
    }

	return true;
}

bool UStaticFunctionLibrary::GetSpaceObjectManager(USpaceObjectManager*& OutSpaceObjectManager)
{
	AJHSGameMode* _gameMode = nullptr;
	if (!GetGameMode(_gameMode))
        return false;

	OutSpaceObjectManager = _gameMode->GetSpaceObjectManager();	
    if (OutSpaceObjectManager == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("GetSpaceObjectManager: SpaceObjectManager is nullptr"));
        return false;
    }

	return true;
}