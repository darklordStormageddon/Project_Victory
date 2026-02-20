// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Asteroid/StaticAsternoidManagerComponent.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"

#include "JHS/Player/SpaceStation.h"

// Sets default values for this component's properties
UStaticAsternoidManagerComponent::UStaticAsternoidManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStaticAsternoidManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	SpaceStation = GetOwner();

	if (bAutoStart)
		StartRound();
}

void UStaticAsternoidManagerComponent::InitSpaceRadius()
{
	AJHSGameMode* InGameMode = nullptr;

	if (!UStaticFunctionLibrary::TryGetGameMode(InGameMode))
		return;

	if (InGameMode && InGameMode->GetSpaceManager())
	{
		SpaceRadius = InGameMode->GetSpaceManager()->GetSpaceRadius() / 1.5f;
	}
}

// Called every frame
void UStaticAsternoidManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStaticAsternoidManagerComponent::SpawnStaticAsteroid()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !GetWorld())
		return;

	if(!SpaceStation)
		return;
	if(!AsteroidInfoArray.IsValidIndex(0))
		return;
	

	const FVector StationPos = SpaceStation ? SpaceStation->GetActorLocation() : FVector::ZeroVector;
	const int32 SpawnNum = FMath::RandRange(SpawnMinNum, SpawnMaxNum);

	for (int32 i = 0; i < SpawnNum; i++)
	{
		bool bValidLocation = false;
		FVector SpawnLocation = FVector::ZeroVector;
		int32 Retries = 0;

		while (!bValidLocation && Retries < MaxRetries)
		{
			Retries++;

			const FVector RandDir = FMath::VRand();

			SpawnLocation = StationPos + RandDir * FMath::RandRange(SpaceStationSaveRadius, SpaceRadius);

			// 정거장 안전 범위 체크
			if (FVector::Dist(SpawnLocation, StationPos) < SpaceStationSaveRadius)
				continue;

			// 이미 생성된 소행성과의 안전 범위 체크
			bool bTooClose = false;
			for (const AActor* Existing : SpawnedAsteroids)
			{
				if (!IsValid(Existing))
					continue;

				if (FVector::Dist(SpawnLocation, Existing->GetActorLocation()) < StaticAsteroidSaveRadius)
				{
					bTooClose = true;
					break;
				}
			}

			if (bTooClose)
				continue;

			bValidLocation = true;
		}

		if (!bValidLocation)
			continue;
		
		const FStaticAsteroidInfo& AsteroidInfo = AsteroidInfoArray[FMath::RandRange(0, AsteroidInfoArray.Num() - 1)];

		const float RandScale = FMath::RandRange(AsteroidInfo.MinSize, AsteroidInfo.MaxSize);
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		SpawnTransform.SetRotation(FMath::VRand().ToOrientationQuat());
		SpawnTransform.SetScale3D(FVector(RandScale));

		AActor* NewAsteroid = GetWorld()->SpawnActor<AActor>(
			AsteroidInfo.StaticAsteroidClass,
			SpawnTransform
		);

		if (NewAsteroid)
			SpawnedAsteroids.Add(NewAsteroid);
	}

	OnStaticAsteroidSpawnComplete.Broadcast();
}

void UStaticAsternoidManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UStaticAsternoidManagerComponent::StartRound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	InitSpaceRadius();
	SpawnStaticAsteroid();
}

void UStaticAsternoidManagerComponent::EndRound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	ClearRoundActors();
}

void UStaticAsternoidManagerComponent::ClearRoundActors()
{
	for (AActor* Asteroid : SpawnedAsteroids)
	{
		if (IsValid(Asteroid))
			Asteroid->Destroy();
	}

	SpawnedAsteroids.Empty();
}