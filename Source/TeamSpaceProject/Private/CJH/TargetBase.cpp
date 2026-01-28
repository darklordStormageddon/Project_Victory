// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/TargetBase.h"

// Sets default values
ATargetBase::ATargetBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATargetBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATargetBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATargetBase::SpawnGarbageSetting()
{
	if (!GarbageTable)
		return;
	if (GarbageRowName.IsNone())
		return;

	const FGarbageInfo* Row =
		GarbageTable->FindRow<FGarbageInfo>(
			GarbageRowName,
			TEXT("GarbageLookup"),
			false
		);

	if (!Row)
		return;

	int Remain = Row->Total_Number;

	for (const auto& Info : Row->GarbageList)
	{
		int Count = FMath::RandRange(
			Info.Min_SpawnNum,
			Info.Max_SpawnNum
		);

		Count = FMath::Min(Count, Remain);

		for (int i = 0; i < Count; ++i)
			Spawn(Info.EnemyGarbage);

		Remain -= Count;
		if (Remain <= 0)
			break;
	}
}

void ATargetBase::Spawn(TSubclassOf<AActor> EnemyGarbage)
{
	if (!EnemyGarbage) return;

	const float Radius = 300.f;

	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.f, Radius);
	FVector SpawnLocation = GetActorLocation() + RandomOffset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(
		EnemyGarbage,
		SpawnLocation,
		FRotator::ZeroRotator,
		Params
	);
}