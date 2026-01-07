// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/SpaceStation.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/SpaceObject/SpaceObjectComponent.h"

#include "JHS/GameControl/JHSGameState.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIPanelDriveSeat.h"

// Sets default values
ASpaceStation::ASpaceStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	_rootComponent->SetupAttachment(RootComponent);

	_spaceObjectComponent = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));
}

// Called when the game starts or when spawned
void ASpaceStation::BeginPlay()
{
	Super::BeginPlay();
	
	AJHSGameState* _outGameState = nullptr;
	UStaticFunctionLibrary::TryGetGameState(_outGameState);
	_gameState = _outGameState;

	UUIManager* _outUIManager = nullptr;
	UStaticFunctionLibrary::TryGetUIManager(_outUIManager);
	_outUIManager->OpenUI(E_UI_TYPE::UIPanelDriveSeat);
}

// Called every frame
void ASpaceStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_gameState != nullptr)
	{
		FSpaceShipData _spaceShipData = _gameState->GetSpaceShipData();
		_spaceShipData.MaxHP += 1.0f;
		_spaceShipData.CurrentHP += 0.3f;

		if (_spaceShipData.MaxHP >= 1500)
		{
			UUIManager* _outUIManager = nullptr;
			UStaticFunctionLibrary::TryGetUIManager(_outUIManager);
			_outUIManager->CloseUI(E_UI_TYPE::UIPanelDriveSeat);
			return;
		}

		_gameState->ChangeSpaceShipData(_spaceShipData);
	}
}