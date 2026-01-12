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
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DistanceCheck())
	{
		LookTarget();

		if(CanFire)
			Fire();
	}
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

