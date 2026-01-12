// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/JHSPlayerBase.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
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
	_gameState->RepairSpaceShip();
}

// Called every frame
void AJHSPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_gameState == nullptr)
		return;

	if (_gameState->GetPlayerCount() > 0)
	{
		for (int32 _playerIdx = 0; _playerIdx < _gameState->GetPlayerCount(); ++_playerIdx)
		{
			float _increaseValue = (_playerIdx + 1) * 0.01f;
			_gameState->IncreasePlayerRadiation(_playerIdx, _increaseValue);
		}
	}
}

// Called to bind functionality to input
void AJHSPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}