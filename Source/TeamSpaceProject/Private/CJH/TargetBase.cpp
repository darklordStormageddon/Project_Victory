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

	const FVector ExplosionCenter = GetActorLocation();
	const float Radius = 150.f;

	// ===== 우주 무중력 환경: 전방향 랜덤 스폰 =====
	FVector SpawnOffset = FMath::VRand() * FMath::FRandRange(20.f, Radius);
	FVector SpawnLocation = ExplosionCenter + SpawnOffset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		EnemyGarbage,
		SpawnLocation,
		FRotator::ZeroRotator,
		Params
	);

	if (!SpawnedActor) return;

	UPrimitiveComponent* Mesh =
		Cast<UPrimitiveComponent>(SpawnedActor->GetRootComponent());

	if (!Mesh) return;

	// ===== 무중력 물리 세팅 =====
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(false);  // 무중력 우주 환경
	Mesh->SetLinearDamping(0.0f);   // 이게 문제! 속도를 계속 줄임
	Mesh->SetAngularDamping(0.0f);  // 회전 마찰 없음 - 계속 회전

	// ===== 핵심: 전방향 충격 (우주에서 퍼지듯이) =====
	FVector ExplosionDir = (SpawnLocation - ExplosionCenter).GetSafeNormal();

	// 힘: 강화된 범위 (더 크게 퍼지도록)
	float DirectionStrength = FMath::FRandRange(10.f, 60.f);
	FVector Impulse = ExplosionDir * DirectionStrength;

	Mesh->AddImpulse(Impulse, NAME_None, false);

	// ===== 회전: 축이 랜덤하고 세기도 랜덤 =====
	FVector TorqueAxis = FMath::VRand();  // 전방향 회전축
	FVector AngularImpulse = TorqueAxis * FMath::FRandRange(60.f, 120.f);

	Mesh->AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);
}