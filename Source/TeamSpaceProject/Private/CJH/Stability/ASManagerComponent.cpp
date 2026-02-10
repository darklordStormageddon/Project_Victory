// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Stability/ASManagerComponent.h"

#include "KSM/Satellite_Base.h"

#include "JHS/Player/SpaceStation.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"

UASManagerComponent::UASManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UASManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	spaceStation = Cast<ASpaceStation>(GetOwner());

	GetSetting();

	if (bAutoStart)
		Artifical_Satellite_Spawn();
}

void UASManagerComponent::StartSpawn()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	Artifical_Satellite_Spawn();
}

void UASManagerComponent::StopSpawn()
{
	if (!GetWorld())
		return;

	GetWorld()->GetTimerManager().ClearTimer(Spawn_TimerHandle);
}

void UASManagerComponent::ClearSpawnedSatellites()
{
	StopSpawn();

	for (const TWeakObjectPtr<ASatellite_Base>& Satellite : SpawnedSatellites)
	{
		if (Satellite.IsValid())
			Satellite->Destroy();
	}

	SpawnedSatellites.Empty();
}

void UASManagerComponent::GetSetting()
{
	AJHSGameMode* InGameMode = nullptr;

	if (!UStaticFunctionLibrary::TryGetGameMode(InGameMode))
		return;

	if (InGameMode && InGameMode->GetSpaceManager())
	{
		Spawn_Distance = InGameMode->GetSpaceManager()->GetSpaceRadius() / 1.5f;
	}
}

void UASManagerComponent::Artifical_Satellite_Spawn()
{
	if (Artifical_Satellite.Num() <= 0)
		return;

	Target_Spawn_Count = FMath::RandRange(min_Spawn, max_Spawn);
	Spawned_Count = 0;

	if (!GetWorld())	
		return;

	GetWorld()->GetTimerManager().ClearTimer(Spawn_TimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		Spawn_TimerHandle,
		this,
		&UASManagerComponent::SpawnNextArtificialSatellite,
		Spawn_Interval,
		true
	);
}

void UASManagerComponent::SpawnNextArtificialSatellite()
{
	if (!spaceStation || Artifical_Satellite.Num() <= 0 || !GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Spawn_TimerHandle);
		return;
	}

	if (Spawned_Count >= Target_Spawn_Count)
	{
		GetWorld()->GetTimerManager().ClearTimer(Spawn_TimerHandle);
		return;
	}

	FVector CenterLocation = spaceStation->GetActorLocation();

	FVector Spawn_Location = FVector(
		FMath::RandRange(CenterLocation.X - Spawn_Distance, CenterLocation.X + Spawn_Distance),
		FMath::RandRange(CenterLocation.Y - Spawn_Distance, CenterLocation.Y + Spawn_Distance),
		FMath::RandRange(CenterLocation.Z - Spawn_Distance, CenterLocation.Z + Spawn_Distance)
	);

	FRotator Spawn_Rotation = FRotator(
		FMath::RandRange(0, 360),
		FMath::RandRange(0, 360),
		FMath::RandRange(0, 360)
	);

	int AS_Num = FMath::RandRange(0, Artifical_Satellite.Num() - 1);

	ASatellite_Base* SpawnedSatellite = GetWorld()->SpawnActor<ASatellite_Base>(Artifical_Satellite[AS_Num], Spawn_Location, Spawn_Rotation);
	if (SpawnedSatellite)
	{
		SpawnedSatellites.Add(SpawnedSatellite);
		OnSatelliteSpawned.Broadcast(SpawnedSatellite);
	}

	Spawned_Count++;
}
