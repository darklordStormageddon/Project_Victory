// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Asteroid/AsteroidComponent.h"
#include "CJH/Asteroid/Asteroid.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UAsteroidComponent::UAsteroidComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAsteroidComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}


// Called every frame
void UAsteroidComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 이미 스폰 중이면 아무것도 안함
	if (bIsSpawning) return;

	CanSpawn();
}

void UAsteroidComponent::CanSpawn()
{
	UWorld* World = GetWorld();

	AActor* Owner = GetOwner();
	if (!World || !Owner) return;

	ShipSpeed = Owner->GetVelocity();

	//UE_LOG(LogTemp, Warning, TEXT("Ship Speed: %.f, %.f, %.f"), ShipSpeed.X, ShipSpeed.Y, ShipSpeed.Z)

	FTimerManager& TimerManager = World->GetTimerManager();

	if (!TimerManager.IsTimerActive(SpawnTimerHandle))
		TimerManager.SetTimer(SpawnTimerHandle, this, &UAsteroidComponent::SpawnMeteor, FMath::RandRange(MinSpawnDelay, MaxSpawnDelay), true);
}
void UAsteroidComponent::SpawnMeteor()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();

	if (!World || !Owner) return;

	// 스폰 플래그 설정
	bIsSpawning = true;

	// 랜덤 방향과 위치
	FVector RandomDirection = FMath::VRand();
	FVector SpawnLocation = Owner->GetActorLocation() + RandomDirection * SpawnDistance;

	float Size = FMath::RandRange(0.1f, 3.0f);
	float Speed = FMath::RandRange(300.f, 1000.f);
	float Health = Size * 100.f;

	// 회전 랜덤
	FRotator SpawnRotation = FRotator(FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f));

	if (AsteroidClasses.Num() == 0)
	{
		bIsSpawning = false;
		return;
	}

	int Index = FMath::RandRange(0, AsteroidClasses.Num() - 1);

	TSubclassOf<AAsteroid> AsteroidClass = AsteroidClasses[Index];
	if (!*AsteroidClass)
	{
		bIsSpawning = false;
		return;
	}

	AAsteroid* Asteroid = World->SpawnActor<AAsteroid>(
		AsteroidClass,
		SpawnLocation,
		SpawnRotation
	);

	bIsSpawning = false;

	if (Asteroid) // Check if Meteor is successfully spawned
	{
		AAsteroid::FAsteroidInfo Info;
		Info.Speed = Speed;
		Info.Size = Size;
		Info.Health = Health;
		Info.Damage = SetDamage(Speed, Size);


		Asteroid->SetAsteroidInfo(
			Info, // 운석의 속도, 크기, 체력, 대미지
			Owner->GetActorLocation(),
			ShipSpeed // 이 컴포넌트의 주인인 우주선 속도
		);
	}
}

float UAsteroidComponent::SetDamage(float Speed, float Size)
{
	float Damage = BaseDamage + (Size * Speed / 100.f);//0.3~40 //10.3~50
	return Damage;
}
