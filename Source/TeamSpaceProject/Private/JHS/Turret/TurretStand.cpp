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
	RemoveCurrentTurret();
	_turretAmmoType = E_AMMO_TYPE::NONE;
}

void ATurretStand::SetTurretType(E_AMMO_TYPE TurretAmmoType)
{
	_turretAmmoType = TurretAmmoType;
}

bool ATurretStand::CanEquipTurret() const
{
	return _turret == nullptr;
}

bool ATurretStand::TryEquipTurret(TObjectPtr<AActor> Turret)
{
	RemoveCurrentTurret();

	if (Turret == nullptr)
		return false;

	_turret = Turret;

	// Turret을 자신의 자식으로 두고 로컬 좌표와 회전값을 0으로 설정
	FAttachmentTransformRules _attachRules = FAttachmentTransformRules::KeepWorldTransform;
	Turret->AttachToComponent(this->GetRootComponent(), _attachRules);
	Turret->SetActorRelativeLocation(FVector::ZeroVector);
	Turret->SetActorRelativeRotation(FRotator::ZeroRotator);
	return true;
}

void ATurretStand::RemoveCurrentTurret()
{
	if (_turret != nullptr)
	{
		_turret->Destroy();
		_turret = nullptr;
	}
}