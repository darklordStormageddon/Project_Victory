// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Turret/TurretStand.h"
#include "JHS/GameControl/CommonEnums.h"

// Sets default values
ATurretStand::ATurretStand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATurretStand::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATurretStand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATurretStand::InitializeTurretStand(TObjectPtr<UTurretStateGroup> TurretStateGroup)
{
	_turretManager = TurretStateGroup;
	_turret = nullptr;
}

bool ATurretStand::TryEquipTurret(TObjectPtr<AActor> Turret, E_AMMO_TYPE AmmoType)
{
	if (Turret == nullptr)
	{
		_turret = nullptr;
		_ammoType = E_AMMO_TYPE::NONE;
		return true;
	}

	if (_turret != nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TurretStand: Turret already equipped at [%s]"), *CommonEnums::GetEnum2FString<E_TURRET_POSITION>(_turretPosition));
		return false;
	}

	_turret = Turret;
	_ammoType = AmmoType;
	return true;
}

E_AMMO_TYPE ATurretStand::GetAmmoType()
{
	if (_turret == nullptr)
		return E_AMMO_TYPE::NONE;

	return _ammoType;
}