// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Asteroid/AsteroidComponent.h"
#include "Engine/World.h"

#include "TimerManager.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/Player/SpaceStation.h"

UAsteroidComponent::UAsteroidComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}
// Called when the game starts
void UAsteroidComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	_ownerActor = GetOwner();

	if (_ownerActor->HasAuthority())
		SpawnAsteroid();
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
	if (Asteroids.Num() >= MaxSpawn)
		return;

	UWorld* World = GetWorld();

	AActor* Owner = Cast<AActor>(UGameplayStatics::GetActorOfClass(World, _asteroidInfo.TargetShip));
	if (!World || !Owner) return;

	ShipSpeed = Owner->GetVelocity();

	//UE_LOG(LogTemp, Warning, TEXT("Ship Speed: %.f, %.f, %.f"), ShipSpeed.X, ShipSpeed.Y, ShipSpeed.Z)

	FTimerManager& TimerManager = World->GetTimerManager();

	if (!TimerManager.IsTimerActive(SpawnTimerHandle))
		TimerManager.SetTimer(SpawnTimerHandle, this, &UAsteroidComponent::SpawnAsteroid, FMath::RandRange(_asteroidInfo.MinSpawnDelay, _asteroidInfo.MaxSpawnDelay), true);
}

void UAsteroidComponent::SpawnAsteroid()
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	AActor* Target = Cast<AActor>(UGameplayStatics::GetActorOfClass(World, _asteroidInfo.TargetShip));

	if (!World || !Owner || !Target) return;

	AJHSGameMode* InGameMode;

	if (!UStaticFunctionLibrary::TryGetGameMode(InGameMode)) return;

	// 스폰 플래그 설정
	bIsSpawning = true;

	// 랜덤 방향과 위치
	FVector RandomDirection = FMath::VRand();
	FVector SpawnLocation = InGameMode->GetSpaceManager()->GetSpaceStation()->GetActorLocation() + RandomDirection * InGameMode->GetSpaceManager()->GetSpaceRadius();

	float Size = FMath::RandRange(_asteroidInfo.MinSize, _asteroidInfo.MaxSize);
	float Speed = FMath::RandRange(_asteroidInfo.MinSpeed, _asteroidInfo.MaxSpeed);
	float Health = Size * _asteroidInfo.Max_HP;

	// 회전 랜덤
	FRotator SpawnRotation = FRotator(FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f));

	if (_asteroidInfo.AsteroidClasses.Num() == 0)
	{
		bIsSpawning = false;
		return;
	}

	int Index = FMath::RandRange(0, _asteroidInfo.AsteroidClasses.Num() - 1);

	TSubclassOf<AAsteroid> AsteroidClass = _asteroidInfo.AsteroidClasses[Index];
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
		FTargetInfo Info;
		Info.Speed = Speed;
		Info.Size = Size;
		Info.Max_HP = Health;
		Info.Attack_Damage = SetDamage(Speed, Size);

		Asteroid->AsteroidComponent = this;
		Asteroid->SetReplicates(true);
		Asteroid->SetReplicateMovement(true);
		Asteroid->AsteroidComponent = this;

		Asteroid->SetAsteroidInfo(
			Info, // 운석의 속도, 크기, 체력, 대미지
			Target->GetActorLocation(),
			ShipSpeed // 이 컴포넌트의 주인인 우주선 속도
		);

		Asteroid->DestroyDistance = InGameMode->GetSpaceManager()->GetSpaceRadius();
		Asteroids.Add(Asteroid);

		if(_asteroidInfo.debugDraw)
			Asteroid->DebugDrawing();
	}
}

float UAsteroidComponent::SetDamage(float Speed, float Size)
{
	float Damage = _asteroidInfo.BaseDamage + (Size * Speed / 100.f);//0.3~40 //10.3~50
	return Damage;
}

void UAsteroidComponent::RemoveAsteroid(AAsteroid* _removeTarget)
{
	if (_removeTarget)
	{
		Asteroids.Remove(_removeTarget);
		//어레이 공간 정리
		Asteroids.Shrink();
	}
}
