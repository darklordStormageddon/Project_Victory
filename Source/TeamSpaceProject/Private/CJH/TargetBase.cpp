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

	// 파편 스폰 위치 (폭발 중심 주변)
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

	// 물리 기본 세팅
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(true);
	Mesh->SetLinearDamping(1.5f);
	Mesh->SetAngularDamping(2.0f);

	// 폭발 방향 (핵심)
	FVector ExplosionDir =
		(SpawnLocation - ExplosionCenter).GetSafeNormal();

	// 힘 조절
	float DirectionStrength = FMath::FRandRange(80.f, 140.f);
	float UpBias = FMath::FRandRange(10.f, 30.f); // 살짝만

	FVector Impulse =
		ExplosionDir * DirectionStrength +
		FVector::UpVector * UpBias;

	Mesh->AddImpulse(Impulse, NAME_None, false);

	// 회전 (방향성 있는 토크)
	FVector TorqueDir = FVector::CrossProduct(ExplosionDir, FMath::VRand());

	FVector AngularImpulse =
		TorqueDir * FMath::FRandRange(100.f, 250.f);

	Mesh->AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);

	// 우주 상태 전환
	FTimerHandle GravityTimer;
	GetWorldTimerManager().SetTimer(
		GravityTimer,
		[Mesh]()
		{
			if (!IsValid(Mesh)) return;

			Mesh->SetEnableGravity(false);
			Mesh->SetLinearDamping(0.05f);
			Mesh->SetAngularDamping(0.05f);
		},
		0.3f,
		false
	);
}