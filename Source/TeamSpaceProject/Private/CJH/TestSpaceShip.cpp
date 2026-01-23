// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/TestSpaceShip.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"

#include "KSM/HealthComponent.h"

#include "JHS/UI/UIManager.h"

// Sets default values
ATestSpaceShip::ATestSpaceShip()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ATestSpaceShip::BeginPlay()
{
	Super::BeginPlay();
	// ...


	HealthComp->OnDamaged.AddDynamic(this, &ATestSpaceShip::OnTakeDamage);
	HealthComp->OnDeath.AddDynamic(this, &ATestSpaceShip::OnDeath);

	FTimerHandle TestDelay;

	GetWorld()->GetTimerManager().SetTimer(
		TestDelay, 
		[this]() {
			UUIManager* _outUIManager = nullptr;

			if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
			return;

			_outUIManager->OpenUI(E_UI_TYPE::UIPanelDriveSeat);
		}, 
		1.f, 
		false);

}

// Called every frame
void ATestSpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATestSpaceShip::OnTakeDamage(float Damage)
{
	if(GetSpaceShipStateGroup())
		_spaceShipStateGroup->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, Damage);
}

void ATestSpaceShip::OnDeath()
{
	this->Destroy();
}

USpaceShipStateGroup* ATestSpaceShip::GetSpaceShipStateGroup()
{
	if (_spaceShipStateGroup)
		return _spaceShipStateGroup;

	AJHSGameState* _outGameState = nullptr;

	if (UStaticFunctionLibrary::TryGetGameState(_outGameState))
	{
		_spaceShipStateGroup = _outGameState->GetSpaceShipStateGroup();
	}

	return _spaceShipStateGroup;
}