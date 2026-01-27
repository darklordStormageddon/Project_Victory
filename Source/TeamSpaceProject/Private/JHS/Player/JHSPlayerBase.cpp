// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/JHSPlayerBase.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/SpaceObject/DriveSeatRader.h"

#include "JHS/UI/UIManager.h"

// Sets default values
AJHSPlayerBase::AJHSPlayerBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJHSPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	// ADriveSeatRader를 칮고 SetSpaceShip 함수 호출
	TArray<AActor*> _driveSeatRaderArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADriveSeatRader::StaticClass(), _driveSeatRaderArray);
	for (AActor* _driveSeatRader : _driveSeatRaderArray)
	{
		ADriveSeatRader* _driveSeatRaderActor = Cast<ADriveSeatRader>(_driveSeatRader);
		_driveSeatRaderActor->SetSpaceShip(this);
	}

	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_gameState = _outGameState;

	UTurretStateGroup* _turretStateGroup = _gameState->GetTurretStateGroup();
	_turretStateGroup->SetInfiniteMagMode(true);
	_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Main, E_AMMO_TYPE::Bullet, nullptr);
	//_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Left, E_AMMO_TYPE::Bullet, nullptr);
	//_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Right, E_AMMO_TYPE::Cannon, nullptr);*/

	/*UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_outUIManager->OpenUI(E_UI_TYPE::UIPanelCollectSeat);*/

	
	//GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &AJHSPlayerBase::FireTurret, 3.0f, false);
}

// Called every frame
void AJHSPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_gameState == nullptr)
		return;

	/*_gameState->GetSpaceShipStateGroup()->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Shield, 0.001f);
	_gameState->GetSpaceShipStateGroup()->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, 0.01f);
	_gameState->GetSpaceShipStateGroup()->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Fuel, 0.05f);

	/*for (int i = 0; i < _gameState->GetPlayerStateGroup()->GetPlayerCount(); i++)
	{
		float _value = (i + 1) * 0.01;
		_gameState->GetPlayerStateGroup()->IncreasePlayerRadiation(i, _value);
	}*/
}

// Called to bind functionality to input
void AJHSPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AJHSPlayerBase::FireTurret()
{
	if (_gameState == nullptr)
		return;

	UTurretStateGroup* _turretStateGroup = _gameState->GetTurretStateGroup();
	if (_turretStateGroup == nullptr)
		return;

	E_TURRET_POSITION _turretPosition = E_TURRET_POSITION::Main;

	_turretStateGroup->TryFireTurret(_turretPosition);

	float _outFireCoolTime = 0.0f;
	if (!_turretStateGroup->TryGetTurretFireInterval(_turretPosition, &_outFireCoolTime))
		return;

	if (_outFireCoolTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &AJHSPlayerBase::FireTurret, _outFireCoolTime, false);
	}
}

void AJHSPlayerBase::AddElement()
{
	if (_gameState == nullptr)
		return;

	UContainerStateGroup* _containerStateGroup = _gameState->GetContainerStateGroup();
	if (_containerStateGroup == nullptr)
		return;


	_elementIndex++;
	int32 _lastIndex = (int32)E_ELEMENT_TYPE::CarbonFiber;
	if (_elementIndex >= _lastIndex)
	{
		_elementIndex %= _lastIndex;
		_isAddMode = !_isAddMode;
	}

	if (_isAddMode)
	{
		_containerStateGroup->AddElement((E_ELEMENT_TYPE)_elementIndex, _elementIndex + 1);
	}
	else
	{
		_containerStateGroup->RemoveElement((E_ELEMENT_TYPE)_elementIndex, _elementIndex + 1);
	}

	GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &AJHSPlayerBase::AddElement, 1.0f, false);
}