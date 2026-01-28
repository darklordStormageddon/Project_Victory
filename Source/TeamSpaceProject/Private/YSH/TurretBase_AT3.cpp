// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/TurretBase_AT3.h"
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

// Sets default values
ATurretBase_AT3::ATurretBase_AT3()
{
	// Set this pawn to call Tick() every frame.
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

	// 머즐 컴포넌트 생성
	LeftMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("LeftMuzzle"));
	LeftMuzzle->SetupAttachment(BarrelMesh);

	RightMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("RightMuzzle"));
	RightMuzzle->SetupAttachment(BarrelMesh);
}

// Called when the game starts or when spawned
void ATurretBase_AT3::BeginPlay()
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
			_cachedGameState->GetTurretStateGroup()->TryEquipTurret(E_TURRET_POSITION::Left, E_AMMO_TYPE::Cannon, this);
		}
	}
}

// Called every frame
void ATurretBase_AT3::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_cachedGameState)
		return;

	// 디버그 시각화
	DrawDebugVisualization();

	// 1. 타겟 탐지 및 추적
	FindAndTrackTarget(DeltaTime);

	// 2. 타겟을 향해서 회전
	if (CurrentTarget)
	{
		RotateTowardsTarget(DeltaTime);

		// 3. 자동 발사 - 좌우 머즐 독립 쿨타임 관리
		LeftMuzzleTimeSinceLastFire += DeltaTime;
		RightMuzzleTimeSinceLastFire += DeltaTime;

		float CurrentFireCoolTime = 0.0f;
		UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();

		if (TurretStateGroup && TurretStateGroup->TryGetTurretFireInterval(TurretPosition, &CurrentFireCoolTime))
		{
			if (IsTargetInLineOfSight())
			{
				// 다음 발사할 머즐의 쿨타임만 체크
				float& CurrentMuzzleCooldown = bIsLeftMuzzleNext ? LeftMuzzleTimeSinceLastFire : RightMuzzleTimeSinceLastFire;

				// 디버그 로그 추가
				static float LastLogTime = 0.0f;
				float CurrentTime = GetWorld()->GetTimeSeconds();
				if (CurrentTime - LastLogTime > 0.5f)  // 0.5초마다 로그
				{
					UE_LOG(LogTemp, Warning, TEXT("AT3 Cooldown Check - Next: %s, Left: %.2f, Right: %.2f, Required: %.2f"),
						bIsLeftMuzzleNext ? TEXT("Left") : TEXT("Right"),
						LeftMuzzleTimeSinceLastFire,
						RightMuzzleTimeSinceLastFire,
						CurrentFireCoolTime);
					LastLogTime = CurrentTime;
				}

				if (CurrentMuzzleCooldown >= CurrentFireCoolTime)
				{
					TryAutoFire();
				}
			}
		}
	}
}

// Called to bind functionality to input
void ATurretBase_AT3::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// 자동 터렛이므로 입력 바인딩 불필요
}

void ATurretBase_AT3::InvalidateCurrentTarget(AActor* DestroyedTarget)
{
	// 파괴된 적이 현재 타겟인 경우 타겟 초기화
	if (CurrentTarget == DestroyedTarget)
	{
		CurrentTarget = nullptr;

		if (bShowDebugRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("ATurretBase_AT3: Target destroyed, searching for new target"));
		}
	}
}

bool ATurretBase_AT3::IsTargetInHemisphere(AActor* Target) const
{
	if (!Target || !bUseHemisphericalDetection)
		return true;

	FVector TurretLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector ToTarget = TargetLocation - TurretLocation;

	// 터렛의 상단 Up 벡터 (회전 고려)
	FVector TurretUpVector = GetActorUpVector();

	float DotProduct = FVector::DotProduct(ToTarget.GetSafeNormal(), TurretUpVector);

	float AngleThreshold = FMath::Sin(FMath::DegreesToRadians(MinimumTargetHeightOffset));

	return DotProduct >= AngleThreshold;
}

void ATurretBase_AT3::FindAndTrackTarget(float DeltaTime)
{
	// 현재 타겟이 유효한지 확인
	if (CurrentTarget)
	{
		if (!IsValid(CurrentTarget) ||
			CurrentTarget->IsPendingKillPending() ||
			!IsTargetInRange() ||
			!IsTargetInHemisphere(CurrentTarget))
		{
			CurrentTarget = nullptr;
		}
	}

	// 타겟이 없으면 새로 찾기
	if (!CurrentTarget && TargetActorClass)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetActorClass, FoundActors);

		float ClosestDistance = DetectionRange;
		AActor* NewTarget = nullptr;

		for (AActor* Actor : FoundActors)
		{
			if (!Actor || !IsValid(Actor) || Actor->IsPendingKillPending())
				continue;

			if (!IsTargetInHemisphere(Actor))
				continue;

			float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				NewTarget = Actor;
				ClosestDistance = Distance;
			}
		}

		if (NewTarget)
		{
			CurrentTarget = NewTarget;
		}
	}
}

void ATurretBase_AT3::RotateTowardsTarget(float DeltaTime)
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

	// Pitch 회전 (상하)
	FRotator CurrentPitchRotation = PitchPivot->GetRelativeRotation();
	float TargetPitch = TargetRotation.Pitch;
	float ClampedTargetPitch = FMath::Clamp(TargetPitch, MinPitch, MaxPitch);

	float DeltaPitch = FMath::FindDeltaAngleDegrees(CurrentPitchRotation.Pitch, ClampedTargetPitch);
	float NewPitch = CurrentPitchRotation.Pitch + FMath::Clamp(DeltaPitch, -RotationSpeed * DeltaTime, RotationSpeed * DeltaTime);

	NewPitch = FMath::Clamp(NewPitch, MinPitch, MaxPitch);
	CurrentPitchRotation.Pitch = NewPitch;
	PitchPivot->SetRelativeRotation(CurrentPitchRotation);
}

void ATurretBase_AT3::TryAutoFire()
{
	UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();
	if (!TurretStateGroup)
		return;

	// 발사할 머즐 선택 (토글 전에 저장)
	USceneComponent* CurrentMuzzle = bIsLeftMuzzleNext ? LeftMuzzle : RightMuzzle;
	bool bFiringLeft = bIsLeftMuzzleNext;

	// 탄약 소비 및 발사 가능 여부 확인
	if (!TurretStateGroup->TryFireTurret(TurretPosition))
		return;

	// ★ 중요: 쿨타임 리셋을 탄약 소비 성공 후 즉시 실행 ★
	if (bFiringLeft)
	{
		LeftMuzzleTimeSinceLastFire = 0.0f;
	}
	else
	{
		RightMuzzleTimeSinceLastFire = 0.0f;
	}

	// 다음 발사할 머즐 토글
	bIsLeftMuzzleNext = !bIsLeftMuzzleNext;

	if (CurrentMuzzle && ProjectileClass && CurrentTarget)
	{
		FVector MuzzleLocation = CurrentMuzzle->GetComponentLocation();

		// 타겟 방향으로 발사 각도 설정
		FVector TargetLocation = CurrentTarget->GetActorLocation();
		FVector Direction = (TargetLocation - MuzzleLocation).GetSafeNormal();
		FRotator FireRotation = Direction.Rotation();

		// 이펙트용 머즐 회전 설정
		FRotator MuzzleRotation = CurrentMuzzle->GetComponentRotation();
		FRotator EffectRotation = MuzzleRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + MuzzleRotation.RotateVector(MuzzleFlashLocationOffset);

		// 발사체 생성
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileClass,
			MuzzleLocation,
			FireRotation,
			SpawnParams
		);

		// 투사체에 터렛 참조 설정 (적 처치 시 알림용)
		if (Projectile)
		{
			Projectile->SetOwningTurret(this);
		}

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

		// 카메라 쉐이크
		if (FireCameraShake)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				PC->ClientStartCameraShake(FireCameraShake);
			}
		}

		// 디버그 로그
		UE_LOG(LogTemp, Warning, TEXT("AT3 Fired: %s Muzzle - Left: %.2f, Right: %.2f"),
			bFiringLeft ? TEXT("Left") : TEXT("Right"),
			LeftMuzzleTimeSinceLastFire,
			RightMuzzleTimeSinceLastFire);
	}
}

bool ATurretBase_AT3::IsTargetInRange() const
{
	if (!CurrentTarget || !IsValid(CurrentTarget) || CurrentTarget->IsPendingKillPending())
		return false;

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > DetectionRange)
		return false;

	if (!IsTargetInHemisphere(CurrentTarget))
		return false;

	return true;
}

bool ATurretBase_AT3::IsTargetInLineOfSight() const
{
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

void ATurretBase_AT3::DrawDebugVisualization()
{
	if (!bShowDebugRange)
		return;

	FVector TurretLocation = GetActorLocation();
	FColor RangeColor = CurrentTarget ? DebugTargetFoundColor : DebugRangeColor;

	if (bUseHemisphericalDetection)
	{
		FVector TurretUpVector = GetActorUpVector();
		FVector HemisphereCenter = TurretLocation;

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

		if (MinimumTargetHeightOffset != 0.0f)
		{
			float AngleRad = FMath::DegreesToRadians(MinimumTargetHeightOffset);
			FVector BoundaryOffset = TurretUpVector * (DetectionRange * FMath::Sin(AngleRad));
			float BoundaryRadius = DetectionRange * FMath::Cos(AngleRad);

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

	if (CurrentTarget && bShowDebugLineOfSight)
	{
		FVector TargetLocation = CurrentTarget->GetActorLocation();

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

	// 좌우 머즐 디버그 표시
	if (LeftMuzzle && RightMuzzle)
	{
		// 왼쪽 머즐
		FVector LeftMuzzleLocation = LeftMuzzle->GetComponentLocation();
		FVector LeftMuzzleForward = LeftMuzzle->GetForwardVector();

		DrawDebugDirectionalArrow(
			GetWorld(),
			LeftMuzzleLocation,
			LeftMuzzleLocation + LeftMuzzleForward * 500.0f,
			50.0f,
			FColor::Cyan,
			false,
			-1.0f,
			0,
			3.0f
		);

		// 오른쪽 머즐
		FVector RightMuzzleLocation = RightMuzzle->GetComponentLocation();
		FVector RightMuzzleForward = RightMuzzle->GetForwardVector();

		DrawDebugDirectionalArrow(
			GetWorld(),
			RightMuzzleLocation,
			RightMuzzleLocation + RightMuzzleForward * 500.0f,
			50.0f,
			FColor::Magenta,
			false,
			-1.0f,
			0,
			3.0f
		);
	}
}

void ATurretBase_AT3::AddYawInput(float YawInputDegPerSec, float DeltaTime)
{
	// 필요시 외부 조작용
}

void ATurretBase_AT3::AddPitchInput(float PitchInputDegPerSec, float DeltaTime)
{
	// 필요시 외부 조작용
}