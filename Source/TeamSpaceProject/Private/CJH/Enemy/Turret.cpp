// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Turret.h"

#include "JHS/GameControl/JHSGameMode.h"

#include "CJH/Enemy/Weapon/Bullet.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	RootComponent = TurretMesh;

	MuzzleArrow1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle1"));
	MuzzleArrow1->SetupAttachment(RootComponent);

	MuzzleArrow2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle2"));
	MuzzleArrow2->SetupAttachment(RootComponent);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle MoveSetHandle;

	GetWorld()->GetTimerManager().SetTimer(
		MoveSetHandle,
		this,
		&ATurret::SetDirection,
		0.1f,
		false
	);
}

void ATurret::SetDirection()
{
	MoveDirection = (_spaceShip->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();
	MoveDirection *= _targetInfo.Speed;

	RotationDirection = FRotator(
		FMath::RandRange(-1.f, 1.f), 
		FMath::RandRange(-1.f, 1.f), 
		FMath::RandRange(-1.f, 1.f)
	);

}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move(DeltaTime);

	if (DistanceCheck(_spawnedInfo.Attack_Range))
	{
		LookTarget();

		if(CanFire)
			Fire();
	}
}

void ATurret::Move(float DeltaTime)
{
	AddActorWorldOffset(_targetInfo.Speed * MoveDirection * DeltaTime, true);

	AddActorWorldRotation(RotateSpeed * RotationDirection * DeltaTime);
}

void ATurret::LookTarget()
{
	FVector TargetLocation = _spaceShip->GetActorLocation();
	FRotator LookRotation = (TargetLocation - GetActorLocation()).Rotation();
	SetActorRotation(LookRotation);
}

void ATurret::Fire()
{
	CanFire = false;

	FTimerHandle FireHandle;

	ABullet* BulletActor1 = GetWorld()->SpawnActor<ABullet>(
		Bullet,
		MuzzleArrow1->GetComponentLocation(),
		MuzzleArrow1->GetComponentRotation()
	);

	ABullet* BulletActor2 = GetWorld()->SpawnActor<ABullet>(
		Bullet,
		MuzzleArrow2->GetComponentLocation(),
		MuzzleArrow2->GetComponentRotation()
	);

	BulletActor1->GetTarget(_spaceShip->GetActorLocation());
	BulletActor2->GetTarget(_spaceShip->GetActorLocation());

	GetWorld()->GetTimerManager().SetTimer(FireHandle, this, &ATurret::EnableFiring, _spawnedInfo.Attack_Speed, false);
}

void ATurret::EnableFiring()
{
	CanFire = true;
}

