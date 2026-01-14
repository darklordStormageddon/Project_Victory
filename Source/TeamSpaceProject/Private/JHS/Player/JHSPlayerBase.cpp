// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/JHSPlayerBase.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/SpaceObject/DriveSeatRader.h"

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
	_gameState->GetSpaceShipStateGroup()->RepairSpaceShip();

	//FireTurret();
}

// Called every frame
void AJHSPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_gameState == nullptr)
		return;

	/*_gameState->GetSpaceShipStateGroup()->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Shield, 0.001f);
	_gameState->GetSpaceShipStateGroup()->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, 0.01f);
	_gameState->GetSpaceShipStateGroup()->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::Fuel, 0.05f);*/

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

	if (!_turretStateGroup->TryFireTurret())
	{
		_turretStateGroup->ReloadTurret();
	}

	float _fireCoolTime = _turretStateGroup->GetTurretFireCoolTime();
	if (_fireCoolTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(_turretFireTimerHandle, this, &AJHSPlayerBase::FireTurret, _fireCoolTime, false);
	}
}