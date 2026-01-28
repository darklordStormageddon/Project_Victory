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
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("ATurretBase_AT1: Failed to get GameState"));
	}
}

void ATurretBase_AT1::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
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

bool ATurretBase_AT1::IsTargetInHemisphere(AActor* Target) const
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

void ATurretBase_AT1::FindAndTrackTarget(float DeltaTime)
{
	// 현재 타겟이 유효한지 확인 (파괴되었거나 범위를 벗어났는지)
	if (CurrentTarget)
	{
		// 타겟이 파괴되었거나 유효하지 않은 경우
		if (!IsValid(CurrentTarget) || CurrentTarget->IsPendingKillPending() || !IsTargetInRange() || !IsTargetInHemisphere(CurrentTarget))
		{
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
				//UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT1: New target acquired: %s (Distance: %.1f)"), 
				//	*CurrentTarget->GetName(), ClosestDistance);
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

FRotator ATurretBase_AT1::GetSpreadRotation(const FRotator& BaseRotation) const
{
	if (!bEnableSpread || SpreadConeAngle <= 0.0f)
	{
		return BaseRotation;
	}

	// 원뿔 내부의 랜덤 포인트 생성
	// 1. 랜덤 각도 (0 ~ 360도)
	float RandomAngle = FMath::FRandRange(0.0f, 360.0f);

	// 2. 원뿔 반경 내의 랜덤 거리 (0 ~ SpreadConeAngle)
	// 균등 분포를 위해 제곱근 사용
	float RandomRadius = FMath::Sqrt(FMath::FRand()) * SpreadConeAngle;

	// 3. 극좌표를 직교좌표로 변환하여 Pitch/Yaw 오프셋 계산
	float OffsetPitch = RandomRadius * FMath::Cos(FMath::DegreesToRadians(RandomAngle));
	float OffsetYaw = RandomRadius * FMath::Sin(FMath::DegreesToRadians(RandomAngle));

	// 4. 기본 회전에 오프셋 추가
	FRotator SpreadRotation = BaseRotation;
	SpreadRotation.Pitch += OffsetPitch;
	SpreadRotation.Yaw += OffsetYaw;

	return SpreadRotation;
}

void ATurretBase_AT1::InvalidateCurrentTarget(AActor* DestroyedTarget)
{
	// 파괴된 적이 현재 타겟인 경우 타겟 초기화
	if (CurrentTarget == DestroyedTarget)
	{
		CurrentTarget = nullptr;

		if (bShowDebugRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT1: Target destroyed, searching for new target"));
		}
	}
}

void ATurretBase_AT1::TryAutoFire()
{
	UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();
	if (!TurretStateGroup)
	{
		//UE_LOG(LogTemp, Error, TEXT("ATurretBase_AT1::TryAutoFire - TurretStateGroup is null"));
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

		// 스프레드 적용된 발사 각도 계산
		FRotator FinalRotation = GetSpreadRotation(MuzzleRotation);

		// 이펙트 위치 및 회전 계산
		FRotator EffectRotation = MuzzleRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + MuzzleRotation.RotateVector(MuzzleFlashLocationOffset);

		// 투사체 생성 (스프레드가 적용된 회전 사용)
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileClass,
			MuzzleLocation,
			FinalRotation,  // 스프레드가 적용된 회전 사용
			SpawnParams
		);

		// 투사체에 타겟 설정
		if (Projectile)
		{
			// 지연 유도를 위해 타겟 설정
			Projectile->SetHomingTarget(CurrentTarget);

			// 터렛 참조 설정 (적 처치 시 알림용)
			Projectile->SetOwningTurret(this);

			//UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT1: Projectile fired at target %s"), *CurrentTarget->GetName());
		}

		// 스프레드 디버그 시각화
		if (bShowSpreadDebug && bEnableSpread)
		{
			// 발사 방향 라인
			DrawDebugLine(
				GetWorld(),
				MuzzleLocation,
				MuzzleLocation + FinalRotation.Vector() * 1000.0f,
				FColor::Orange,
				false,
				0.5f,
				0,
				2.0f
			);

			// 원뿔 시각화
			DrawDebugCone(
				GetWorld(),
				MuzzleLocation,
				MuzzleRotation.Vector(),
				500.0f,
				FMath::DegreesToRadians(SpreadConeAngle),
				FMath::DegreesToRadians(SpreadConeAngle),
				12,
				FColor::Yellow,
				false,
				0.5f,
				0,
				1.0f
			);
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
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->ClientStartCameraShake(FireCameraShake);
			}
		}
	}
}

bool ATurretBase_AT1::IsTargetInRange() const
{
	// 타겟이 null이거나 유효하지 않으면 false
	if (!CurrentTarget || !IsValid(CurrentTarget) || CurrentTarget->IsPendingKillPending())
		return false;

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= DetectionRange;
}

bool ATurretBase_AT1::IsTargetInLineOfSight() const
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

void ATurretBase_AT1::DrawDebugVisualization()
{
	if (!bShowDebugRange)
		return;

	FVector TurretLocation = GetActorLocation();
	FColor RangeColor = CurrentTarget ? DebugTargetFoundColor : DebugRangeColor;

	if (bUseHemisphericalDetection)
	{
		// 터렛의 회전을 반영한 반구 시각화
		FVector TurretUpVector = GetActorUpVector();

		// 반구의 중심점 (Up 벡터 방향으로 오프셋)
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