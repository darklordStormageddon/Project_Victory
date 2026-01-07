// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/JHSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "JHS/UI/UIManager.h"

AJHSGameMode::AJHSGameMode()
{
	_spaceObjectManager = CreateDefaultSubobject<USpaceObjectManager>(TEXT("SpaceObjectManager"));
	_uiManager = CreateDefaultSubobject<UUIManager>(TEXT("UIManager"));
}

void AJHSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AJHSGameMode::BeginPlay()
{
	Super::BeginPlay();
}

ASpaceStation* AJHSGameMode::GetSpaceStation()
{
	// 없으면 스캔해서 찾기
	if (_spaceStation == nullptr)
	{
		UWorld* _world = GetWorld();
		if (!_world)
		{
			UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: _world is nullptr"));
			return nullptr;
		}

		TArray<AActor*> _foundSpaceStationArray;
		UGameplayStatics::GetAllActorsOfClass(_world, ASpaceStation::StaticClass(), _foundSpaceStationArray);
		if (_foundSpaceStationArray.Num() > 0)
		{
			_spaceStation = Cast<ASpaceStation>(_foundSpaceStationArray[0]);
		}
	}

	if (_spaceStation == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: SpaceStation is not found"));
		return nullptr;
	}

	return _spaceStation;
}
