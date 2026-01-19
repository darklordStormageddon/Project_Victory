// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/EnemyBase.h"

#include "JHS/GameControl/JHSGameMode.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	FireComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireComp"));
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



// 플레이어와의 거리 체크
bool AEnemyBase::DistanceCheck(float _condition)
{
	if (!_spaceShip)
		return false;

	float Distance = FVector::Dist(GetActorLocation(), _spaceShip->GetActorLocation());

	if (Distance <= _condition)
		return true;

	return false;
}

void AEnemyBase::SetTargetShip(TSubclassOf<AActor> Targetenemy) 
{ 
	_spaceShip = UGameplayStatics::GetActorOfClass(GetWorld(), Targetenemy);
}