// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 충돌 컴포넌트 생성
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComponent;

	// 메시 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 발사체 이동 컴포넌트 생성
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// 생존 주기 설정
	InitialLifeSpan = LifeSpan;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 충돌 이벤트 바인딩
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	// 자신이나 발사한 Actor는 무시
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// 적중 위치와 법선 벡터 계산
	FVector HitLocation = Hit.ImpactPoint;
	FRotator HitRotation = Hit.ImpactNormal.Rotation();

	// 디버그 시각화
	if (bShowDebugHit)
	{
		DrawDebugSphere(GetWorld(), HitLocation, 50.0f, 12, FColor::Red, false, 2.0f, 0, 3.0f);
		DrawDebugDirectionalArrow(GetWorld(), HitLocation, HitLocation + Hit.ImpactNormal * 100.0f, 25.0f, FColor::Green, false, 2.0f, 0, 3.0f);

		UE_LOG(LogTemp, Warning, TEXT("Projectile hit: %s at location: %s"), *OtherActor->GetName(), *HitLocation.ToString());
	}

	// 데미지 적용
	UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

	// 적중 이펙트 생성
	SpawnHitEffect(HitLocation, HitRotation);

	// 발사체 즉시 파괴
	Destroy();
}

void AProjectile::SpawnHitEffect(const FVector& HitLocation, const FRotator& HitRotation)
{
	// 파티클 이펙트 생성
	if (HitEffect)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitEffect,
			HitLocation,
			HitRotation,
			HitEffectScale,
			true,
			EPSCPoolMethod::AutoRelease,
			true
		);

		if (PSC)
		{
			PSC->bAutoDestroy = true;
			PSC->SecondsBeforeInactive = 0.0f;
		}
	}

	// 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitLocation);
	}
}