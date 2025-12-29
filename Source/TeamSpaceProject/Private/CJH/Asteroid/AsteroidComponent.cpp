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

	UWorld* World = GetWorld();
	if (!World) return;

	// 타이머가 이미 활성화되어 있지 않으면 타이머 설정 (중복 설정 방지)
	FTimerManager& TimerManager = World->GetTimerManager();
	if (!TimerManager.IsTimerActive(SpawnTimerHandle))
	{
		TimerManager.SetTimer(SpawnTimerHandle, this, &UAsteroidComponent::SpawnMeteor, SpawnDelay, true);
	}
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

	if (MeteorClasses.Num() == 0)
	{
		bIsSpawning = false;
		return;
	}

	int32 Index = FMath::RandRange(0, MeteorClasses.Num() - 1);
	TSubclassOf<AAsteroid> MeteorClass = MeteorClasses[Index];
	if (!*MeteorClass)
	{
		bIsSpawning = false;
		return;
	}

	AAsteroid* Meteor = World->SpawnActor<AAsteroid>(
		MeteorClass,
		SpawnLocation,
		SpawnRotation
	);

	bIsSpawning = false;

	if (Meteor) // Check if Meteor is successfully spawned
	{
		// AAsteroid 내에 정의된 FMeteorInfo를 명시적으로 사용하여 구조체 생성
		AAsteroid::FMeteorInfo Info;
		Info.Speed = Speed;
		Info.Size = Size;
		Info.Health = Health;
		Info.Damage = SetDamage(Speed, Size);

		Meteor->SetMeteorInfo(
			Info, // 운석의 속도, 크기, 체력, 대미지
			Owner->GetActorLocation(),
			Owner->GetVelocity() // 이 컴포넌트의 주인인 우주선 속도
		);
	}
	// (참고) 타이머를 계속 반복시키려면 ClearTimer를 호출하지 않습니다.
	// 일회성으로만 스폰하려면 아래 주석을 해제하세요.
	// World->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

float UAsteroidComponent::SetDamage(float Speed, float Size)
{
	float Damage = BaseDamage + (Size * Speed / 100.f);//0.3~40 //10.3~50
	return Damage;
}
