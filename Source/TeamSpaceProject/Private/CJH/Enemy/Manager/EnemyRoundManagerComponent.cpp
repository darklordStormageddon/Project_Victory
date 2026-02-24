// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Enemy/Manager/EnemyRoundManagerComponent.h"

#include "CJH/Enemy/Base/EnemyBase.h"
#include "CJH/Enemy/Manager/EnemySpawnComponent.h"
#include "CJH/Enemy/Manager/GarbageEnemySpawnComponent.h"
#include "CJH/Stability/ASManagerComponent.h"
#include "KSM/Satellite_Base.h"

UEnemyManagerComponent::UEnemyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	SatelliteManager = GetOwner()->FindComponentByClass<UASManagerComponent>();

	if (SatelliteManager)
	{
		SatelliteManager->OnSatelliteSpawned.AddDynamic(this, &UEnemyManagerComponent::HandleSatelliteSpawned);
	}
}

void UEnemyManagerComponent::StartRound(int32 Round)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	CurrentRound = Round;
}

void UEnemyManagerComponent::EndRound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	ClearRoundActors();
	CurrentRound = 0;
}

void UEnemyManagerComponent::HandleSatelliteSpawned(ASatellite_Base* Satellite)
{
	if (!Satellite || CurrentRound <= 0)
		return;

	SpawnedSatellites.Add(Satellite);
	SpawnEnemiesForSatellite(Satellite);
}

void UEnemyManagerComponent::SpawnForExistingSatellites()
{
	if (!SatelliteManager || CurrentRound <= 0)
		return;

	for (const TWeakObjectPtr<ASatellite_Base>& Satellite : SatelliteManager->GetSpawnedSatellites())
	{
		if (!Satellite.IsValid())
			continue;

		SpawnedSatellites.Add(Satellite);
		SpawnEnemiesForSatellite(Satellite.Get());
	}
}

FRoundEnemySettings UEnemyManagerComponent::GetRoundSettings(int32 Round) const
{
	if (RoundSettings.Num() <= 0)
		return FRoundEnemySettings();

	const int32 Index = FMath::Clamp(Round - 1, 0, RoundSettings.Num() - 1);
	return RoundSettings[Index];
}

void UEnemyManagerComponent::BuildSpawnList(const FRoundEnemySettings& Settings, int32 Round, TArray<TSubclassOf<AEnemyBase>>& OutSpawnList) const
{
	OutSpawnList.Reset();

	for (const FEnemySpawnGroup& Group : Settings.Groups)
	{
		if (!Group.EnemyTypes)
			continue;

		const int32 Count = FMath::RandRange(Group.MinCount, Group.MaxCount);
		for (int32 i = 0; i < Count; ++i)
		{
			OutSpawnList.Add(Group.EnemyTypes);
		}
	}

	const int32 MaxCap = (Round >= 5)
		? FixedEnemyCountAfterRound5
		: FMath::Min(Settings.MaxTotal, FixedEnemyCountAfterRound5);

	while (OutSpawnList.Num() > MaxCap)
	{
		OutSpawnList.RemoveAt(OutSpawnList.Num() - 1);
	}

	if (Round >= 5 && OutSpawnList.Num() < MaxCap)
	{
		TArray<TSubclassOf<AEnemyBase>> AllTypes;
		for (const FEnemySpawnGroup& Group : Settings.Groups)
		{
			if (Group.EnemyTypes)
			{
				AllTypes.Add(Group.EnemyTypes);
			}
		}

		if (AllTypes.Num() > 0)
		{
			while (OutSpawnList.Num() < MaxCap)
			{
				const int32 TypeIndex = FMath::RandRange(0, AllTypes.Num() - 1);
				OutSpawnList.Add(AllTypes[TypeIndex]);
			}
		}
	}
}

void UEnemyManagerComponent::SpawnEnemiesForSatellite(ASatellite_Base* Satellite)
{
	if (!Satellite)
		return;

	FRoundEnemySettings Settings = GetRoundSettings(CurrentRound);
	TArray<TSubclassOf<AEnemyBase>> SpawnList;
	BuildSpawnList(Settings, CurrentRound, SpawnList);

	if (SpawnList.Num() <= 0)
		return;

	if (UGarbageEnemySpawnComponent* GarbageComponent = Satellite->FindComponentByClass<UGarbageEnemySpawnComponent>())
	{
		GarbageComponent->SpawnEnemies(SpawnList);
		return;
	}

	if (UEnemySpawnComponent* SpawnComponent = Satellite->FindComponentByClass<UEnemySpawnComponent>())
	{
		SpawnComponent->SpawnEnemies(SpawnList);
	}
}

void UEnemyManagerComponent::ClearRoundActors()
{
	if (SatelliteManager)
	{
		for (const TWeakObjectPtr<ASatellite_Base>& Satellite : SatelliteManager->GetSpawnedSatellites())
		{
			if (!Satellite.IsValid())
				continue;

			if (UGarbageEnemySpawnComponent* GarbageComponent = Satellite->FindComponentByClass<UGarbageEnemySpawnComponent>())
				GarbageComponent->ClearSpawnedEnemies();
			else if (UEnemySpawnComponent* SpawnComponent = Satellite->FindComponentByClass<UEnemySpawnComponent>())
				SpawnComponent->ClearSpawnedEnemies();
		}

		SatelliteManager->ClearSpawnedSatellites();
	}

	SpawnedSatellites.Empty();
}
