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
	NewScale = FVector(Size, Size, Size);

	SetActorScale3D(NewScale);

	Super::SetInfo();

}