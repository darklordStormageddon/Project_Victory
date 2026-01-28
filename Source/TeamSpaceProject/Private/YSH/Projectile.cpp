// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/Projectile.h"
#include "YSH/TurretBase_AT1.h"
#include "YSH/TurretBase_AT2.h"
#include "YSH/TurretBase_AT3.h"
#include "KSM/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
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

	// 기본적으로 유도 비활성화 (지연 후 활성화)
	ProjectileMovement->bIsHomingProjectile = false;

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

	// 지연 가속이 활성화된 경우 초기 속도를 느리게 설정
	if (bEnableDelayedAcceleration && ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = LaunchSpeed;
		ProjectileMovement->MaxSpeed = BoostSpeed;
		ProjectileMovement->Velocity = GetActorForwardVector() * LaunchSpeed;

		if (bShowDebugAcceleration)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Projectile launched with slow speed: %.2f"), LaunchSpeed);
		}
	}
}

void AProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 부스트 이펙트 정리
	CleanupBoostEffect();

	Super::EndPlay(EndPlayReason);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TimeAlive를 항상 증가 (가속과 유도 모두 이 값을 사용)
	TimeAlive += DeltaTime;

	// 지연 가속 처리
	if (bEnableDelayedAcceleration)
	{
		UpdateAcceleration(DeltaTime);
	}

	// 지연 유도 처리
	if (bEnableDelayedHoming)
	{
		UpdateHoming(DeltaTime);
	}
}

void AProjectile::SetHomingTarget(AActor* Target)
{
	HomingTarget = Target;

	if (bShowDebugHoming)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Projectile homing target set: %s"), Target ? *Target->GetName() : TEXT("None"));
	}
}

void AProjectile::UpdateAcceleration(float DeltaTime)
{
	if (!ProjectileMovement)
		return;

	// 지연 시간 경과 후 급가속 시작
	if (TimeAlive >= AccelerationDelay)
	{
		// 부스트 활성화 처리 (속도와 무관하게 한 번만 실행)
		if (!bHasBoosted)
		{
			bHasBoosted = true;

			// ===== Niagara 부스트 이펙트 생성 =====
			if (BoostEffectNiagara)
			{
				ActiveBoostEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
					BoostEffectNiagara,
					MeshComponent ? MeshComponent : RootComponent,
					NAME_None,
					BoostEffectLocationOffset,
					BoostEffectRotationOffset,
					BoostEffectScale,
					EAttachLocation::KeepRelativeOffset,
					false,
					ENCPoolMethod::None,
					true,
					true
				);

				if (ActiveBoostEffect)
				{
					//UE_LOG(LogTemp, Warning, TEXT("✓ Boost Niagara Effect Spawned!"));
				}
			}

			// 부스트 사운드 재생
			if (BoostSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, BoostSound, GetActorLocation());
			}

			if (bShowDebugAcceleration)
			{
				float CurrentSpeed = ProjectileMovement->Velocity.Size();
				//UE_LOG(LogTemp, Warning, TEXT("Projectile boost activated at %.2fs! Speed: %.2f -> %.2f"), 
				//	TimeAlive, CurrentSpeed, BoostSpeed);
			}
		}

		// 가속도 적용 (이미 목표 속도에 도달했어도 계속 실행)
		float CurrentSpeed = ProjectileMovement->Velocity.Size();
		if (CurrentSpeed < BoostSpeed)
		{
			float NewSpeed = FMath::FInterpConstantTo(CurrentSpeed, BoostSpeed, DeltaTime, AccelerationRate);
			ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * NewSpeed;

			// 디버그 시각화
			if (bShowDebugAcceleration)
			{
				FVector Start = GetActorLocation();
				FVector End = Start + ProjectileMovement->Velocity.GetSafeNormal() * 200.0f;
				DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, -1.0f, 0, 3.0f);

				FVector BoostEffectWorldLocation = Start + GetActorRotation().RotateVector(BoostEffectLocationOffset);
				DrawDebugSphere(GetWorld(), BoostEffectWorldLocation, 10.0f, 8, FColor::Cyan, false, -1.0f, 0, 2.0f);

				DrawDebugString(
					GetWorld(),
					GetActorLocation() + FVector(0, 0, 50),
					FString::Printf(TEXT("Speed: %.0f"), NewSpeed),
					nullptr,
					FColor::Yellow,
					0.0f,
					true
				);
			}
		}
	}
	else if (bShowDebugAcceleration)
	{
		// 가속 전 대기 시간 시각화
		DrawDebugString(
			GetWorld(),
			GetActorLocation() + FVector(0, 0, 50),
			FString::Printf(TEXT("Waiting: %.2fs"), AccelerationDelay - TimeAlive),
			nullptr,
			FColor::Cyan,
			0.0f,
			true
		);
	}
}

void AProjectile::UpdateHoming(float DeltaTime)
{
	if (!ProjectileMovement || !HomingTarget)
		return;

	// 지연 시간이 경과했는지 확인
	if (TimeAlive < HomingDelay)
	{
		return;
	}

	// 유도 활성화 시점 이펙트 (한 번만)
	if (!bHomingActivated)
	{
		bHomingActivated = true;

		if (bUseHomingAcceleration)
		{
			// 커스텀 유도 사용
			if (bShowDebugHoming)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Projectile homing activated (Custom)!"));
			}
		}
		else
		{
			// UProjectileMovementComponent의 기본 유도 사용
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;

			if (HomingTarget)
			{
				USceneComponent* TargetComponent = HomingTarget->GetRootComponent();
				if (TargetComponent)
				{
					ProjectileMovement->HomingTargetComponent = TargetComponent;
				}
			}

			if (bShowDebugHoming)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Projectile homing activated (Built-in)!"));
			}
		}
	}

	// 커스텀 유도 로직
	if (bUseHomingAcceleration && HomingTarget)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = HomingTarget->GetActorLocation();
		FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();

		// 현재 속도 방향
		FVector CurrentVelocity = ProjectileMovement->Velocity;
		float CurrentSpeed = CurrentVelocity.Size();
		FVector CurrentDirection = CurrentVelocity.GetSafeNormal();

		// 목표 방향으로 회전 (급격하게 휘어지도록)
		FVector NewDirection = FMath::VInterpConstantTo(
			CurrentDirection,
			ToTarget,
			DeltaTime,
			FMath::DegreesToRadians(HomingTurnSpeed)
		);

		// 새로운 속도 설정 (방향 변경 + 가속)
		float NewSpeed = FMath::Min(CurrentSpeed + HomingAcceleration * DeltaTime, ProjectileMovement->MaxSpeed);
		ProjectileMovement->Velocity = NewDirection * NewSpeed;

		// 디버그 시각화
		if (bShowDebugHoming)
		{
			FVector Start = CurrentLocation;

			// 타겟까지의 라인
			DrawDebugLine(GetWorld(), Start, TargetLocation, FColor::Red, false, -1.0f, 0, 2.0f);

			// 현재 이동 방향
			DrawDebugLine(GetWorld(), Start, Start + NewDirection * 300.0f, FColor::Green, false, -1.0f, 0, 3.0f);

			// 타겟 위치 표시
			DrawDebugSphere(GetWorld(), TargetLocation, 30.0f, 8, FColor::Red, false, -1.0f, 0, 2.0f);

			DrawDebugString(
				GetWorld(),
				GetActorLocation() + FVector(0, 0, 80),
				FString::Printf(TEXT("Homing Active\nSpeed: %.0f"), NewSpeed),
				nullptr,
				FColor::Green,
				0.0f,
				true
			);
		}
	}
}

void AProjectile::CleanupBoostEffect()
{
	if (ActiveBoostEffect && ActiveBoostEffect->IsValidLowLevel())
	{
		// 이펙트 즉시 비활성화 및 파괴
		ActiveBoostEffect->Deactivate();
		ActiveBoostEffect->DestroyComponent();
		ActiveBoostEffect = nullptr;

		//UE_LOG(LogTemp, Log, TEXT("✓ Boost effect cleaned up"));
	}
}

void AProjectile::SetOwningTurret(AActor* Turret)
{
	OwningTurret = Turret;
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

	bool bTargetKilled = false;

	// ===== HealthComponent를 사용한 데미지 적용 (Bullet 로직 적용) =====
	if (UHealthComponent* HealthComp = OtherActor->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->TakeDamage(Damage);
		UE_LOG(LogTemp, Log, TEXT("Projectile dealt %.1f damage to %s via HealthComponent"), Damage, *OtherActor->GetName());

		// 체력이 0 이하인지 확인
		if (HealthComp->CurrentHealth <= 0.0f)
		{
			bTargetKilled = true;
			UE_LOG(LogTemp, Warning, TEXT("Projectile killed target: %s"), *OtherActor->GetName());
		}
	}
	// ===== 기존 UGameplayStatics::ApplyDamage 방식 (백업) =====
	else
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		UE_LOG(LogTemp, Log, TEXT("Projectile dealt %.1f damage to %s via UGameplayStatics"), Damage, *OtherActor->GetName());
	}

	// 적을 처치한 경우 터렛에게 알림
	if (bTargetKilled && OwningTurret.IsValid())
	{
		// AT1, AT2, AT3 모두 처리
		if (ATurretBase_AT1* Turret1 = Cast<ATurretBase_AT1>(OwningTurret.Get()))
		{
			Turret1->InvalidateCurrentTarget(OtherActor);
		}
		else if (ATurretBase_AT2* Turret2 = Cast<ATurretBase_AT2>(OwningTurret.Get()))
		{
			Turret2->InvalidateCurrentTarget(OtherActor);
		}
		else if (ATurretBase_AT3* Turret3 = Cast<ATurretBase_AT3>(OwningTurret.Get()))
		{
			Turret3->InvalidateCurrentTarget(OtherActor);
		}
	}

	// 부스트 이펙트 정리 (적중 이펙트 생성 전)
	CleanupBoostEffect();

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
			PSC->bAutoDestroy = false;
			PSC->SecondsBeforeInactive = 0.0f;
		}
	}

	// 사운드 재생
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitLocation);
	}
}