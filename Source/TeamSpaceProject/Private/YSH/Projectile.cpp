// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/Projectile.h"
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
			UE_LOG(LogTemp, Warning, TEXT("Projectile launched with slow speed: %.2f"), LaunchSpeed);
		}
	}
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
		UE_LOG(LogTemp, Warning, TEXT("Projectile homing target set: %s"), Target ? *Target->GetName() : TEXT("None"));
	}
}

void AProjectile::UpdateAcceleration(float DeltaTime)
{
	if (!ProjectileMovement || bHasBoosted)
		return;

	// 지연 시간 경과 후 급가속 시작
	if (TimeAlive >= AccelerationDelay)
	{
		float CurrentSpeed = ProjectileMovement->Velocity.Size();

		// 목표 속도에 도달하지 않았다면 가속
		if (CurrentSpeed < BoostSpeed)
		{
			// 급가속 시작 시 한 번만 이펙트/사운드 재생
			if (!bHasBoosted)
			{
				bHasBoosted = true;

				// ===== Niagara 부스트 이펙트 생성 (우선순위 1) =====
				if (BoostEffectNiagara)
				{
					UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
						BoostEffectNiagara,
						MeshComponent ? MeshComponent : RootComponent,
						NAME_None,
						BoostEffectLocationOffset,      // Location
						BoostEffectRotationOffset,      // Rotation
						BoostEffectScale,               // Scale (FVector)
						EAttachLocation::KeepRelativeOffset,
						true,                           // bAutoDestroy
						ENCPoolMethod::AutoRelease,     // PoolingMethod
						true,                           // bAutoActivate
						true                            // bPreCullCheck
					);

					if (NiagaraComp)
					{
						UE_LOG(LogTemp, Warning, TEXT("✓ Boost Niagara Effect Spawned! Offset: %s, Rotation: %s"),
							*BoostEffectLocationOffset.ToString(),
							*BoostEffectRotationOffset.ToString());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("✗ Failed to spawn Boost Niagara Effect!"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("⚠ No Boost Effect assigned (Neither Niagara nor Cascade)"));
				}

				// 부스트 사운드 재생
				if (BoostSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, BoostSound, GetActorLocation());
					UE_LOG(LogTemp, Warning, TEXT("✓ Boost Sound Played!"));
				}

				if (bShowDebugAcceleration)
				{
					UE_LOG(LogTemp, Warning, TEXT("Projectile boost activated! Accelerating from %.2f to %.2f"), CurrentSpeed, BoostSpeed);
				}
			}

			// 가속도 적용
			float NewSpeed = FMath::FInterpConstantTo(CurrentSpeed, BoostSpeed, DeltaTime, AccelerationRate);
			ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * NewSpeed;

			// 디버그 시각화
			if (bShowDebugAcceleration)
			{
				FVector Start = GetActorLocation();
				FVector End = Start + ProjectileMovement->Velocity.GetSafeNormal() * 200.0f;
				DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, -1.0f, 0, 3.0f);

				// 부스트 이펙트 위치 디버그
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
		// 유도 전 대기 시간 디버그 시각화
		if (bShowDebugHoming)
		{
			DrawDebugString(
				GetWorld(),
				GetActorLocation() + FVector(0, 0, 80),
				FString::Printf(TEXT("Homing Wait: %.2fs"), HomingDelay - TimeAlive),
				nullptr,
				FColor::Magenta,
				0.0f,
				true
			);
		}
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
				UE_LOG(LogTemp, Warning, TEXT("Projectile homing activated (Custom)!"));
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
				UE_LOG(LogTemp, Warning, TEXT("Projectile homing activated (Built-in)!"));
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