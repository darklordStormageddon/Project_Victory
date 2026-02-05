// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASManager.h"

#include "KSM/Satellite_Base.h"

#include "JHS/Player/SpaceStation.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"


// Called when the game starts or when spawned
void AASManager::BeginPlay()
{
	Super::BeginPlay();


	// 폐기물 위성 스폰 함수 호출
	if (HasAuthority())
	{
		GetSetting();

		Artifical_Satellite_Spawn();
	}
}

void AASManager::GetSetting()
{
	AJHSGameMode* InGameMode = nullptr;
	
	if (!UStaticFunctionLibrary::TryGetGameMode(InGameMode))
		return;
	
	if (InGameMode)
	{
		spaceStation = InGameMode->GetSpaceManager()->GetSpaceStation();
		Spawn_Distance = InGameMode->GetSpaceManager()->GetSpaceRadius()/1.5;
	}
}

void AASManager::Artifical_Satellite_Spawn()
{
	if (Artifical_Satellite.Num() <= 0)
		return;

	Target_Spawn_Count = FMath::RandRange(min_Spawn, max_Spawn);
	Spawned_Count = 0;

	GetWorld()->GetTimerManager().ClearTimer(Spawn_TimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		Spawn_TimerHandle,
		this,
		&AASManager::SpawnNextArtificialSatellite,
		Spawn_Interval,
		true
	);
}

void AASManager::SpawnNextArtificialSatellite()
{
	if (!spaceStation || Artifical_Satellite.Num() <= 0)
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

	GetWorld()->SpawnActor<ASatellite_Base>(Artifical_Satellite[AS_Num], Spawn_Location, Spawn_Rotation);

	Spawned_Count++;
}

