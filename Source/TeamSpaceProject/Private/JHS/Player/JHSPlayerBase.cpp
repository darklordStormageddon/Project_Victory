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

	//FireTurret();
}

// Called every frame
void AJHSPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_gameState == nullptr)
		return;

	//_gameState->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, 0.01f);
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

	if (!_gameState->TryFireTurret())
	{
		_gameState->ReloadTurret();
	}

	float _fireCoolTime = _gameState->GetTurretFireCoolTime();
	if (_fireCoolTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(_turretFireTimerHandle, this, &AJHSPlayerBase::FireTurret, _fireCoolTime, false);
	}
}