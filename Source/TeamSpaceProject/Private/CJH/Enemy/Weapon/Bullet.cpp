// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Weapon/Bullet.h"

#include "KSM/HealthComponent.h"

// Sets default values
ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;

	Collision->InitSphereRadius(5.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetGenerateOverlapEvents(true);

	// 기본은 전부 무시
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 맞출 대상만 Overlap
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(
		this,
		&ABullet::OnBulletOverlap
	);
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveToTarget(DeltaTime);

	BulletLifeTime -= DeltaTime;

	if(BulletLifeTime <= 0.0f)
		Destroy();
}

void ABullet::MoveToTarget(float DeltaTime)
{
	SetActorLocation(GetActorLocation() + Direction * Speed * DeltaTime, true);
}

void ABullet::OnBulletOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor || OtherActor == _owner || !_owner)
		return;

	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
		Health->TakeDamage(Damage);

	Destroy(); // 맞으면 사라짐
}


