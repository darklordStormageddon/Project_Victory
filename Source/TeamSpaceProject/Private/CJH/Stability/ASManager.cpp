// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASManager.h"

#include "KSM/Satellite_Base.h"

#include "JHS/Player/SpaceStation.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"


// Called when the game starts or when spawned
void AASManager::BeginPlay()
{
	Super::BeginPlay();

	GetSetting();

	// 폐기물 위성 스폰 함수 호출
	Artifical_Satellite_Spawn();
}

void AASManager::GetSetting()
{
	AJHSGameMode* InGameMode = nullptr;
	
	if (!UStaticFunctionLibrary::TryGetGameMode(InGameMode)) return;
	
	if (InGameMode)
	{
		spaceStation = InGameMode->GetSpaceStation();
		Spawn_Distance = InGameMode->GetSpaceRadius();
	}
}

void AASManager::Artifical_Satellite_Spawn()
{
	// 폐기물 스폰 개수 랜덤 결정
	int Spawn_Num = FMath::RandRange(min_Spawn, max_Spawn);

	FVector CenterLocation = spaceStation->GetActorLocation();

	for (int i = 0; i < Spawn_Num; i++)
	{
		// 스폰 위치와 회전 랜덤 결정
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

		// 폐기물 위성 코어 스폰
		if(Artifical_Satellite.Num() <= 0)
			return;

		int AS_Num = FMath::RandRange(0, Artifical_Satellite.Num() - 1);

		ASatellite_Base* Spawned_AS = GetWorld()->SpawnActor<ASatellite_Base>(Artifical_Satellite[AS_Num], Spawn_Location, Spawn_Rotation);
		
		/*Spawned_AS->Info.Speed = FMath::RandRange(min_Speed, max_Speed);
		FVector RandDir = FVector(
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(-1.f, 1.f)
		);
		Spawned_AS->Info.Direction = RandDir.GetSafeNormal();*/
	}
}

