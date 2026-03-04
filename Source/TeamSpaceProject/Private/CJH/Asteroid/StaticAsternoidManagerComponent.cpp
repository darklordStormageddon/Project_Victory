// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Asteroid/StaticAsternoidManagerComponent.h"

#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Player/SpaceStation.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UStaticAsternoidManagerComponent::UStaticAsternoidManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UStaticAsternoidManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	SpaceStation = GetOwner();

	if (bAutoStart)
		StartRound();
}

void UStaticAsternoidManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStaticAsternoidManagerComponent, RepSpawnDataList);
}

void UStaticAsternoidManagerComponent::InitSpaceRadius()
{
	USpaceManager* SpaceManager = nullptr;

	if (!UStaticFunctionLibrary::TryGetSpaceManager(SpaceManager))
		return;

	SpawnRadius = SpaceManager->GetSpaceRadius() / 1.5f;
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

	if (!SpaceStation)
		return;
	if (!AsteroidInfoArray.IsValidIndex(0))
		return;

	const FVector StationPos = SpaceStation->GetActorLocation();
	const int32 SpawnNum = FMath::RandRange(SpawnMinNum, SpawnMaxNum);

	// 기존 스폰 데이터 초기화
	RepSpawnDataList.Empty();

	for (int32 i = 0; i < SpawnNum; i++)
	{
		bool bValidLocation = false;
		FVector SpawnLocation = FVector::ZeroVector;
		int32 Retries = 0;

		while (!bValidLocation && Retries < MaxRetries)
		{
			Retries++;

			const FVector RandDir = FMath::VRand();
			SpawnLocation = StationPos + RandDir * FMath::RandRange(SpaceStationSaveRadius, SpawnRadius);

			// 정거장 안전 범위 체크
			if (FVector::DistSquared(SpawnLocation, StationPos) < SpaceStationSaveRadius * SpaceStationSaveRadius)
				continue;

			// 이미 생성된 소행성과의 안전 범위 체크
			bool bTooClose = false;
			for (const AActor* Existing : SpawnedAsteroids)
			{
				if (!IsValid(Existing))
					continue;

				if (FVector::DistSquared(SpawnLocation, Existing->GetActorLocation()) < StaticAsteroidSaveRadius * StaticAsteroidSaveRadius)
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

		// 서버: 직접 스폰
		AActor* NewAsteroid = GetWorld()->SpawnActor<AActor>(
			AsteroidInfo.StaticAsteroidClass,
			SpawnTransform
		);

		if (NewAsteroid)
		{
			SpawnedAsteroids.Add(NewAsteroid);

			// 클라이언트 스폰용 데이터 저장
			FStaticAsteroidSpawnData Data;
			Data.AsteroidClass = AsteroidInfo.StaticAsteroidClass;
			Data.Location      = SpawnLocation;
			Data.Rotation      = SpawnTransform.GetRotation().Rotator();
			Data.Scale         = RandScale;
			RepSpawnDataList.Add(Data);
		}
	}

	// RepSpawnDataList가 복제되면 클라이언트의 OnRep_SpawnDataList가 호출됨
	OnStaticAsteroidSpawnComplete.Broadcast();
}

// 클라이언트에서 RepSpawnDataList 수신 시 호출
void UStaticAsternoidManagerComponent::OnRep_SpawnDataList()
{
	if (!GetWorld())
		return;

	// 기존 클라이언트 소행성 제거 후 재스폰
	for (AActor* A : ClientSpawnedAsteroids)
	{
		if (IsValid(A))
			A->Destroy();
	}
	ClientSpawnedAsteroids.Empty();

	for (const FStaticAsteroidSpawnData& Data : RepSpawnDataList)
	{
		SpawnAsteroidOnClient(Data);
	}
}

void UStaticAsternoidManagerComponent::SpawnAsteroidOnClient(const FStaticAsteroidSpawnData& Data)
{
	if (!Data.AsteroidClass || !GetWorld())
		return;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Data.Location);
	SpawnTransform.SetRotation(Data.Rotation.Quaternion());
	SpawnTransform.SetScale3D(FVector(Data.Scale));

	AActor* NewAsteroid = GetWorld()->SpawnActor<AActor>(
		Data.AsteroidClass,
		SpawnTransform
	);

	if (NewAsteroid)
		ClientSpawnedAsteroids.Add(NewAsteroid);
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

	for (AActor* Asteroid : ClientSpawnedAsteroids)
	{
		if (IsValid(Asteroid))
			Asteroid->Destroy();
	}
	ClientSpawnedAsteroids.Empty();

	RepSpawnDataList.Empty();
}