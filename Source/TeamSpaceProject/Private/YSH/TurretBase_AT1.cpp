// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/TurretBase_AT1.h"
#include "YSH/Projectile.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ATurretBase_AT1::ATurretBase_AT1()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	YawPivot = CreateDefaultSubobject<USceneComponent>(TEXT("YawPivot"));
	YawPivot->SetupAttachment(Root);

	PitchPivot = CreateDefaultSubobject<USceneComponent>(TEXT("PitchPivot"));
	PitchPivot->SetupAttachment(YawPivot);

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(YawPivot);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(PitchPivot);

	MainMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("MainMuzzle"));
	MainMuzzle->SetupAttachment(BarrelMesh);
}

void ATurretBase_AT1::BeginPlay()
{
	Super::BeginPlay();

	// GameState 캐싱
	AJHSGameState* TempGameState = nullptr;
	if (UStaticFunctionLibrary::TryGetGameState(TempGameState))
	{
		_cachedGameState = TempGameState;

		// TurretStateGroup의 TryEquipTurret 사용
		if (_cachedGameState->GetTurretStateGroup())
		{
			_cachedGameState->GetTurretStateGroup()->TryEquipTurret(E_TURRET_POSITION::Left, E_AMMO_TYPE::Missile);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase_AT1: Failed to get GameState"));
	}
}

void ATurretBase_AT1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_cachedGameState)
		return;

	// 디버그 시각화
	DrawDebugVisualization();

	// 1. 타겟 탐색 및 추적
	FindAndTrackTarget(DeltaTime);

	// 2. 타겟이 있으면 회전
	if (CurrentTarget)
	{
		RotateTowardsTarget(DeltaTime);

		// 3. 자동 발사 - TurretStateGroup의 TryGetTurretFireInterval 사용
		TimeSinceLastFire += DeltaTime;

		float CurrentFireCoolTime = 0.0f;
		UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();

		if (TurretStateGroup && TurretStateGroup->TryGetTurretFireInterval(TurretPosition, &CurrentFireCoolTime))
		{
			if (TimeSinceLastFire >= CurrentFireCoolTime && IsTargetInLineOfSight())
			{
				TryAutoFire();
				TimeSinceLastFire = 0.0f;
			}
		}
	}
}

void ATurretBase_AT1::FindAndTrackTarget(float DeltaTime)
{
	// 현재 타겟이 유효한지 확인
	if (CurrentTarget && !IsTargetInRange())
	{
		CurrentTarget = nullptr;
	}

	// 타겟이 없으면 새로 찾기
	if (!CurrentTarget && TargetActorClass)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetActorClass, FoundActors);

		float ClosestDistance = DetectionRange;

		for (AActor* Actor : FoundActors)
		{
			if (!Actor)
				continue;

			float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				CurrentTarget = Actor;
				ClosestDistance = Distance;
			}
		}
	}
}

void ATurretBase_AT1::RotateTowardsTarget(float DeltaTime)
{
	if (!CurrentTarget || !YawPivot || !PitchPivot)
		return;

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector TurretLocation = GetActorLocation();
	FVector Direction = TargetLocation - TurretLocation;

	if (Direction.IsNearlyZero())
		return;

	FRotator TargetRotation = Direction.Rotation();

	// Yaw 회전 (좌우)
	FRotator CurrentYawRotation = YawPivot->GetRelativeRotation();
	float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYawRotation.Yaw, TargetRotation.Yaw);
	float NewYaw = CurrentYawRotation.Yaw + FMath::Clamp(DeltaYaw, -RotationSpeed * DeltaTime, RotationSpeed * DeltaTime);

	CurrentYawRotation.Yaw = NewYaw;
	YawPivot->SetRelativeRotation(CurrentYawRotation);

	// Pitch 회전 (상하) - 부호 제거
	FRotator CurrentPitchRotation = PitchPivot->GetRelativeRotation();
	float TargetPitch = TargetRotation.Pitch; // 부호 반전 제거!
	float ClampedTargetPitch = FMath::Clamp(TargetPitch, MinPitch, MaxPitch);

	float DeltaPitch = FMath::FindDeltaAngleDegrees(CurrentPitchRotation.Pitch, ClampedTargetPitch);
	float NewPitch = CurrentPitchRotation.Pitch + FMath::Clamp(DeltaPitch, -RotationSpeed * DeltaTime, RotationSpeed * DeltaTime);

	NewPitch = FMath::Clamp(NewPitch, MinPitch, MaxPitch);
	CurrentPitchRotation.Pitch = NewPitch;
	PitchPivot->SetRelativeRotation(CurrentPitchRotation);
}

void ATurretBase_AT1::TryAutoFire()
{
	UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();
	if (!TurretStateGroup)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase_AT1::TryAutoFire - TurretStateGroup is null"));
		return;
	}

	// TurretStateGroup의 TryFireTurret 사용 - 탄약 소비 및 발사 가능 여부 확인
	if (!TurretStateGroup->TryFireTurret(TurretPosition))
	{
		// 발사 실패 (탄약 부족 등)
		return;
	}

	if (MainMuzzle && ProjectileClass && CurrentTarget)
	{
		FVector MuzzleLocation = MainMuzzle->GetComponentLocation();
		FRotator MuzzleRotation = MainMuzzle->GetComponentRotation();

		// 이펙트 위치 및 회전 계산
		FRotator EffectRotation = MuzzleRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + MuzzleRotation.RotateVector(MuzzleFlashLocationOffset);

		// 투사체 생성
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileClass,
			MuzzleLocation,
			MuzzleRotation,
			SpawnParams
		);

		// 투사체에 타겟 설정 (추적 기능)
		if (Projectile)
		{
			// Projectile에 타겟 설정 - HomingTargetComponent 사용
			UProjectileMovementComponent* ProjectileMovement = Projectile->FindComponentByClass<UProjectileMovementComponent>();
			if (ProjectileMovement)
			{
				ProjectileMovement->bIsHomingProjectile = true;
				ProjectileMovement->HomingAccelerationMagnitude = 5000.0f; // 추적 가속도

				// 타겟의 루트 컴포넌트를 추적 대상으로 설정
				USceneComponent* TargetComponent = CurrentTarget->GetRootComponent();
				if (TargetComponent)
				{
					ProjectileMovement->HomingTargetComponent = TargetComponent;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT1: Projectile fired at target %s"), *CurrentTarget->GetName());
		}

		bIsLeftMuzzleNext = !bIsLeftMuzzleNext;

		// Muzzle Flash 이펙트
		if (MuzzleFlashEffect)
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlashEffect,
				EffectLocation,
				EffectRotation,
				FVector(MuzzleFlashScale),
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

		// 발사 사운드
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, EffectLocation);
		}

		// 카메라 셰이크
		if (FireCameraShake)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				PC->ClientStartCameraShake(FireCameraShake);
			}
		}
	}
}

bool ATurretBase_AT1::IsTargetInRange() const
{
	if (!CurrentTarget)
		return false;

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= DetectionRange;
}

bool ATurretBase_AT1::IsTargetInLineOfSight() const
{
	if (!CurrentTarget)
		return false;

	FVector Start = GetActorLocation();
	FVector End = CurrentTarget->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	return !HitResult.bBlockingHit || HitResult.GetActor() == CurrentTarget;
}

void ATurretBase_AT1::DrawDebugVisualization()
{
	if (!bShowDebugRange)
		return;

	FVector TurretLocation = GetActorLocation();

	// 1. 감지 범위 구체 그리기
	FColor RangeColor = CurrentTarget ? DebugTargetFoundColor : DebugRangeColor;
	DrawDebugSphere(
		GetWorld(),
		TurretLocation,
		DetectionRange,
		32,
		RangeColor,
		false,
		-1.0f,
		0,
		2.0f
	);

	// 2. 타겟이 있을 경우 시야선 그리기
	if (CurrentTarget && bShowDebugLineOfSight)
	{
		FVector TargetLocation = CurrentTarget->GetActorLocation();

		// 타겟까지의 라인
		DrawDebugLine(
			GetWorld(),
			TurretLocation,
			TargetLocation,
			DebugLineOfSightColor,
			false,
			-1.0f,
			0,
			3.0f
		);

		// 타겟 위치에 작은 구체
		DrawDebugSphere(
			GetWorld(),
			TargetLocation,
			50.0f,
			12,
			DebugLineOfSightColor,
			false,
			-1.0f,
			0,
			2.0f
		);

		// 타겟 이름 표시
		DrawDebugString(
			GetWorld(),
			TargetLocation + FVector(0, 0, 100),
			CurrentTarget->GetName(),
			nullptr,
			FColor::White,
			0.0f,
			true
		);
	}

	// 3. 포탑 방향 표시
	if (MainMuzzle)
	{
		FVector MuzzleLocation = MainMuzzle->GetComponentLocation();
		FVector MuzzleForward = MainMuzzle->GetForwardVector();

		DrawDebugDirectionalArrow(
			GetWorld(),
			MuzzleLocation,
			MuzzleLocation + MuzzleForward * 500.0f,
			50.0f,
			FColor::Cyan,
			false,
			-1.0f,
			0,
			3.0f
		);
	}
}

void ATurretBase_AT1::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// 자동 포탑이므로 입력 바인딩 불필요
}

void ATurretBase_AT1::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATurretBase_AT1::AddYawInput(float YawInputDegPerSec, float DeltaTime)
{
	// 필요시 외부 제어용
}

void ATurretBase_AT1::AddPitchInput(float PitchInputDegPerSec, float DeltaTime)
{
	// 필요시 외부 제어용
}