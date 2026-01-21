// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Manager/EnemyManagerComponent.h"

#include "JHS/GameControl/JHSGameMode.h"

#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "CJH/Enemy/Base/SpawnedEnemyBase.h"

UEnemyManagerComponent::UEnemyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner = GetOwner();

	if (!_owner)
		return;

	_gameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());

	if (!_gameMode)
		return;

	_spaceRadius = _gameMode->GetSpaceRadius();
	_spaceStation = _gameMode->GetSpaceStation();	
}

void UEnemyManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UEnemyManagerComponent::DeleteAllEnemy()
{
	_spawnedEnemies.Empty();
	_spawnedEnemies.Shrink();
}