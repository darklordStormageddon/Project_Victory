// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Enemy/Manager/EnemySpawnComponent.h"

#include "CJH/Enemy/Base/EnemyBase.h"
#include "KSM/Satellite_Base.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/SpaceManager.h"

UEnemySpawnComponent::UEnemySpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemySpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner = GetOwner();

	if (!_owner)
		return;

	_gameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());

	if (!_gameMode || !_gameMode->GetSpaceManager())
		return;

	_spaceRadius = _gameMode->GetSpaceManager()->GetSpaceRadius();
	_spaceStation = _gameMode->GetSpaceManager()->GetSpaceStation();
}

void UEnemySpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UEnemySpawnComponent::SpawnEnemies(const TArray<TSubclassOf<AEnemyBase>>& EnemiesToSpawn)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
		return;

	if (EnemiesToSpawn.Num() <= 0)
		return;

	TArray<USceneComponent*> AttachPoints;
	if (ASatellite_Base* Satellite = Cast<ASatellite_Base>(GetOwner()))
	{
		if (Satellite->SceneChild1) AttachPoints.Add(Satellite->SceneChild1);
		if (Satellite->SceneChild2) AttachPoints.Add(Satellite->SceneChild2);
		if (Satellite->SceneChild3) AttachPoints.Add(Satellite->SceneChild3);
		if (Satellite->SceneChild4) AttachPoints.Add(Satellite->SceneChild4);
		if (Satellite->SceneChild5) AttachPoints.Add(Satellite->SceneChild5);
		if (Satellite->SceneChild6) AttachPoints.Add(Satellite->SceneChild6);
		if (Satellite->SceneChild7) AttachPoints.Add(Satellite->SceneChild7);
		if (Satellite->SceneChild8) AttachPoints.Add(Satellite->SceneChild8);
	}

	if (AttachPoints.Num() == 0)
	{
		if (USceneComponent* Root = GetOwner()->GetRootComponent())
			AttachPoints.Add(Root);
	}

	if (AttachPoints.Num() == 0)
		return;

	int32 AttachIndex = 0;

	for (const TSubclassOf<AEnemyBase>& EnemyClass : EnemiesToSpawn)
	{
		if (!EnemyClass)
			continue;

		USceneComponent* AttachPoint = AttachPoints[AttachIndex % AttachPoints.Num()];
		const FTransform SpawnTransform = AttachPoint->GetComponentTransform();

		AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnTransform);
		if (!NewEnemy)
			continue;

		NewEnemy->AttachToComponent(AttachPoint, FAttachmentTransformRules::KeepWorldTransform);
		NewEnemy->SetOwner(GetOwner());
		NewEnemy->SetTargetShip(_spaceShip);
		NewEnemy->OwnerGET(_owner);
		NewEnemy->ComponentGET(this);
		NewEnemy->EnemyComponent = this;

		_spawnedEnemies.Add(NewEnemy);
		OnEnemySpawned(NewEnemy);

		AttachIndex++;
	}
}

void UEnemySpawnComponent::ClearSpawnedEnemies()
{
	for (AEnemyBase* Enemy : _spawnedEnemies)
	{
		if (IsValid(Enemy))
			Enemy->Destroy();
	}

	_spawnedEnemies.Empty();
	_spawnedEnemies.Shrink();
}

void UEnemySpawnComponent::DeleteAllEnemy()
{
	ClearSpawnedEnemies();
}

void UEnemySpawnComponent::OnEnemySpawned(AEnemyBase* NewEnemy)
{
}
