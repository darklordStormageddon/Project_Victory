// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/TurretBase_AT2.h"
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

ATurretBase_AT2::ATurretBase_AT2()
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

void ATurretBase_AT2::BeginPlay()
{
	Super::BeginPlay();

	// GameState 캐싱
	AJHSGameState* TempGameState = nullptr;
	if (UStaticFunctionLibrary::TryGetGameState(TempGameState))
	{
		_cachedGameState = TempGameState;
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("ATurretBase_AT2: Failed to get GameState"));
	}
}

void ATurretBase_AT2::Tick(float DeltaTime)
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

void ATurretBase_AT2::InvalidateCurrentTarget(AActor* DestroyedTarget)
{
	// 파괴된 적이 현재 타겟인 경우 타겟 초기화
	if (CurrentTarget == DestroyedTarget)
	{
		CurrentTarget = nullptr;

		if (bShowDebugRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT2: Target destroyed, searching for new target"));
		}
	}
}

bool ATurretBase_AT2::IsTargetInHemisphere(AActor* Target) const
{
	if (!Target || !bUseHemisphericalDetection)
		return true;

	FVector TurretLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector ToTarget = TargetLocation - TurretLocation;

	// 터렛의 로컬 Up 벡터 (회전 적용)
	FVector TurretUpVector = GetActorUpVector();

	float DotProduct = FVector::DotProduct(ToTarget.GetSafeNormal(), TurretUpVector);

	float AngleThreshold = FMath::Sin(FMath::DegreesToRadians(MinimumTargetHeightOffset));

	return DotProduct >= AngleThreshold;
}

void ATurretBase_AT2::FindAndTrackTarget(float DeltaTime)
{
	// 현재 타겟이 유효한지 확인 (파괴되었거나 범위를 벗어났는지)
	if (CurrentTarget)
	{
		// 타겟이 파괴되었거나 유효하지 않거나 범위를 벗어났거나 반구 밖에 있는 경우
		if (!IsValid(CurrentTarget) ||
			CurrentTarget->IsPendingKillPending() ||
			!IsTargetInRange() ||
			!IsTargetInHemisphere(CurrentTarget))
		{
			if (bShowDebugRange)
			{
				//UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT2: Target lost - %s"), 
				//	CurrentTarget ? *CurrentTarget->GetName() : TEXT("NULL"));
			}
			CurrentTarget = nullptr;
		}
	}

	// 타겟이 없으면 새로 찾기 (조건을 분리하여 같은 프레임에서 실행 가능하도록)
	if (!CurrentTarget && TargetActorClass)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetActorClass, FoundActors);

		float ClosestDistance = DetectionRange;
		AActor* NewTarget = nullptr;

		for (AActor* Actor : FoundActors)
		{
			// 유효하지 않거나 파괴 예정인 액터는 무시
			if (!Actor || !IsValid(Actor) || Actor->IsPendingKillPending())
				continue;

			// 반구형 감지 체크
			if (!IsTargetInHemisphere(Actor))
				continue;

			float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				NewTarget = Actor;
				ClosestDistance = Distance;
			}
		}

		// 새로운 타겟 설정
		if (NewTarget)
		{
			CurrentTarget = NewTarget;

			if (bShowDebugRange)
			{
				//UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT2: New target acquired: %s (Distance: %.1f)"),
				//	*CurrentTarget->GetName(), ClosestDistance);
			}
		}
	}
}

void ATurretBase_AT2::RotateTowardsTarget(float DeltaTime)
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

void ATurretBase_AT2::TryAutoFire()
{
	UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();
	if (!TurretStateGroup)
	{
		//UE_LOG(LogTemp, Error, TEXT("ATurretBase_AT2::TryAutoFire - TurretStateGroup is null"));
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

		// 타겟 방향으로 발사 각도 계산
		FVector TargetLocation = CurrentTarget->GetActorLocation();
		FVector Direction = (TargetLocation - MuzzleLocation).GetSafeNormal();
		FRotator FireRotation = Direction.Rotation();

		// 이펙트는 머즐 기준 회전 사용
		FRotator MuzzleRotation = MainMuzzle->GetComponentRotation();
		FRotator EffectRotation = MuzzleRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + MuzzleRotation.RotateVector(MuzzleFlashLocationOffset);

		// 투사체 생성 - 타겟 방향으로 발사
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileClass,
			MuzzleLocation,
			FireRotation,  // 타겟 방향으로 발사!
			SpawnParams
		);

		// 투사체에 터렛 참조 설정 (적 처치 시 알림용)
		if (Projectile)
		{
			Projectile->SetOwningTurret(this);
		}

		bIsLeftMuzzleNext = !bIsLeftMuzzleNext;

		// Muzzle Flash 이펙트 - AutoRelease로 자동 정리되도록 수정
		if (MuzzleFlashEffect)
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlashEffect,
				EffectLocation,
				EffectRotation,
				FVector(MuzzleFlashScale),
				true,  // bAutoDestroy = true
				EPSCPoolMethod::AutoRelease,
				true   // bAutoActivate = true
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
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->ClientStartCameraShake(FireCameraShake);
			}
		}
	}
}

bool ATurretBase_AT2::IsTargetInRange() const
{
	// 타겟이 null이거나 유효하지 않으면 false
	if (!CurrentTarget || !IsValid(CurrentTarget) || CurrentTarget->IsPendingKillPending())
		return false;

	// 거리 체크
	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > DetectionRange)
		return false;

	// 반구형 감지 체크
	if (!IsTargetInHemisphere(CurrentTarget))
		return false;

	return true;
}

bool ATurretBase_AT2::IsTargetInLineOfSight() const
{
	// 타겟이 null이거나 유효하지 않으면 false
	if (!CurrentTarget || !IsValid(CurrentTarget) || CurrentTarget->IsPendingKillPending())
		return false;

	FVector Start = GetActorLocation();
	FVector End = CurrentTarget->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	return !HitResult.bBlockingHit || HitResult.GetActor() == CurrentTarget;
}

void ATurretBase_AT2::DrawDebugVisualization()
{
	if (!bShowDebugRange)
		return;

	FVector TurretLocation = GetActorLocation();
	FColor RangeColor = CurrentTarget ? DebugTargetFoundColor : DebugRangeColor;

	if (bUseHemisphericalDetection)
	{
		// 터렛의 회전을 반영한 반구 시각화
		FVector TurretUpVector = GetActorUpVector();

		// 반구의 중심점
		FVector HemisphereCenter = TurretLocation;

		// 반구를 원과 호로 시각화
		DrawDebugSphere(
			GetWorld(),
			HemisphereCenter,
			DetectionRange,
			32,
			RangeColor,
			false,
			-1.0f,
			0,
			2.0f
		);

		// 터렛의 Up 방향 표시 (반구의 중심 방향)
		DrawDebugDirectionalArrow(
			GetWorld(),
			TurretLocation,
			TurretLocation + TurretUpVector * DetectionRange,
			100.0f,
			FColor::Blue,
			false,
			-1.0f,
			0,
			3.0f
		);

		// 감지 경계면 표시 (최소 각도 threshold)
		if (MinimumTargetHeightOffset != 0.0f)
		{
			float AngleRad = FMath::DegreesToRadians(MinimumTargetHeightOffset);
			FVector BoundaryOffset = TurretUpVector * (DetectionRange * FMath::Sin(AngleRad));
			float BoundaryRadius = DetectionRange * FMath::Cos(AngleRad);

			// 경계 평면의 법선 벡터 계산을 위한 직교 벡터
			FVector RightVector = GetActorRightVector();
			FVector ForwardVector = GetActorForwardVector();

			DrawDebugCircle(
				GetWorld(),
				TurretLocation + BoundaryOffset,
				BoundaryRadius,
				32,
				FColor::Cyan,
				false,
				-1.0f,
				0,
				2.0f,
				RightVector,
				ForwardVector
			);
		}

		// 반구의 경계선 추가 시각화 (적도)
		DrawDebugCircle(
			GetWorld(),
			TurretLocation,
			DetectionRange,
			32,
			FColor::Orange,
			false,
			-1.0f,
			0,
			2.0f,
			GetActorRightVector(),
			GetActorForwardVector()
		);
	}
	else
	{
		// 기존 구형 범위
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
	}

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

		// 타겟 이름과 거리 표시
		float Distance = FVector::Dist(TurretLocation, TargetLocation);
		DrawDebugString(
			GetWorld(),
			TargetLocation + FVector(0, 0, 100),
			FString::Printf(TEXT("%s\nDist: %.0f"), *CurrentTarget->GetName(), Distance),
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