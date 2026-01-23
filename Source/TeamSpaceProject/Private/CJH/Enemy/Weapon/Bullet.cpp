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
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);

	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	// ⭐ 핵심
	Collision->SetNotifyRigidBodyCollision(true);
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentHit.AddDynamic(this, &ABullet::OnHit);
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

void ABullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == _owner || !_owner)
		return;

	if (!HitParticle)
		return;
	UE_LOG(LogTemp, Warning, TEXT("%s"), *OtherActor->GetName())

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitParticle,
		Hit.ImpactPoint,
		Hit.ImpactNormal.Rotation()
	);

	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
		Health->TakeDamage(Damage);

	Destroy(); // 맞으면 사라짐
}


