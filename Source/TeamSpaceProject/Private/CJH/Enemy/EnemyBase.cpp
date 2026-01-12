// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/EnemyBase.h"

#include "JHS/GameControl/JHSGameMode.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DelayHandle;

	GetWorld()->GetTimerManager().SetTimer(
		DelayHandle,
		this,
		&AEnemyBase::SetInfo,
		0.1f, 
		false
	);
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	delayTime += DeltaTime;

	if (delayTime >= 1.0f)
	{
		DistanceCheck();
		delayTime = 0.0f;
	}
}

void AEnemyBase::SetInfo()
{
	// 적 크기 구조체에 따라 크기 설정
	FVector NewScale = FVector(_spawnedInfo.Size, _spawnedInfo.Size, _spawnedInfo.Size);

	SetActorScale3D(NewScale);

	// 적 크기에 비례하여 능력치 증감
	_spawnedInfo.Max_HP *= _spawnedInfo.Size;
	_spawnedInfo.Attack_Damage *= _spawnedInfo.Size;
	_spawnedInfo.Value *= _spawnedInfo.Size;

	_spawnedInfo.Current_HP = _spawnedInfo.Max_HP;
}
// 플레이어와의 거리 체크
bool AEnemyBase::DistanceCheck()
{
	if (!_spaceShip)
		return false;

	float Distance = FVector::Dist(GetActorLocation(), _spaceShip->GetActorLocation());

	if (Distance <= _spawnedInfo.Attack_Range)
		return true;

	return false;
}

void AEnemyBase::SetTargetShip(TSubclassOf<AActor> Target) 
{ 
	_spaceShip = UGameplayStatics::GetActorOfClass(GetWorld(), Target); 
}