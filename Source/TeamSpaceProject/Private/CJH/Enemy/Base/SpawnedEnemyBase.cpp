// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/SpawnedEnemyBase.h"

void ASpawnedEnemyBase::BeginPlay()
{
	FTimerHandle DelayHandle;

	GetWorld()->GetTimerManager().SetTimer(
		DelayHandle,
		this,
		&ASpawnedEnemyBase::SetInfo,
		0.01f,
		false
	);
}

void ASpawnedEnemyBase::SetInfo()
{
	// 적 크기 구조체에 따라 크기 설정
	FVector NewScale = FVector(Size, Size, Size);

	SetActorScale3D(NewScale);

	// 적 크기에 비례하여 능력치 증감
	_spawnedInfo.Max_HP *= Size;
	_spawnedInfo.Attack_Damage *= Size;
	_spawnedInfo.Value *= Size;

	_spawnedInfo.Current_HP = _spawnedInfo.Max_HP;
}