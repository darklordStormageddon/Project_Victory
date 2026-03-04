// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/GarbageEnemyBase.h"
#include "KSM/HealthComponent.h"
#include "Net/UnrealNetwork.h"

AGarbageEnemyBase::AGarbageEnemyBase()
{
	// 클라이언트 보간은 매 프레임 필요
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	bReplicates = true;
	SetReplicateMovement(false);
	NetUpdateFrequency = 20.0f;
	MinNetUpdateFrequency = 10.0f;
}

void AGarbageEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	const FVector StartLoc = GetActorLocation();
	ClientSmoothLoc = StartLoc;
	ClientTargetLoc = StartLoc;
	bClientLocInit  = false;

	if (!HasAuthority())
		return;

	RepLocation = StartLoc;

	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(
		DelayHandle, this, &AGarbageEnemyBase::SetInfo, 0.01f, false);
}

void AGarbageEnemyBase::SetInfo()
{
	if (!HasAuthority())
		return;

	_targetInfo.Size = FMath::RandRange(_spawnedInfo.MinSize, _spawnedInfo.MaxSize);
	NewScale = FVector(_targetInfo.Size, _targetInfo.Size, _targetInfo.Size);
	Super::SetInfo();
}

// SpawnComponent가 0.05초마다 호출 - 서버에서 궤도 위치에 직접 배치
void AGarbageEnemyBase::SetOrbitPosition(const FVector& InPos, float TangentSpeed)
{
	if (!HasAuthority())
		return;

	SetActorLocation(InPos, false, nullptr, ETeleportType::None);

	if (!RotationAxis.IsNearlyZero())
	{
		const FQuat SpinQuat(
			GetActorQuat().RotateVector(RotationAxis).GetSafeNormal(),
			FMath::DegreesToRadians(SpinSpeed * 0.05f));
		SetActorRotation(SpinQuat * GetActorQuat());
	}

	RepLocation = InPos;
}

// 클라이언트에서 RepLocation 수신 시 호출
void AGarbageEnemyBase::OnRep_GarbageState()
{
	const FVector NewTarget = FVector(RepLocation);

	if (!bClientLocInit)
	{
		ClientSmoothLoc   = NewTarget;
		ClientTargetLoc   = NewTarget;
		ClientPrevLoc     = NewTarget;
		ClientInterpAlpha = 1.0f;
		ClientInterpSpeed = 0.0f;
		bClientLocInit    = true;
		return;
	}

	const float Dist = FVector::Dist(ClientSmoothLoc, NewTarget);

	// 즉시 스냅 조건: 스폰 직후 또는 비정상적으로 먼 거리
	if (Dist > 1500.0f)
	{
		ClientSmoothLoc   = NewTarget;
		ClientTargetLoc   = NewTarget;
		ClientPrevLoc     = NewTarget;
		ClientInterpAlpha = 1.0f;
		ClientInterpSpeed = 0.0f;
		return;
	}

	// 보간 시작점 = 현재 스무스 위치
	ClientPrevLoc     = ClientSmoothLoc;
	ClientTargetLoc   = NewTarget;
	ClientInterpAlpha = 0.0f;

	// 속도 = 이번 OnRep까지의 거리 / 갱신 주기
	// 이 속도가 다음 OnRep까지 고정 → 일정한 속도 보장
	ClientInterpSpeed = (Dist > KINDA_SMALL_NUMBER) ? (Dist / 0.05f) : 0.0f;
}

void AGarbageEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
		return;

	if (!bClientLocInit)
	{
		const FVector Loc = GetActorLocation();
		ClientSmoothLoc   = Loc;
		ClientTargetLoc   = Loc;
		ClientPrevLoc     = Loc;
		ClientInterpAlpha = 1.0f;
		bClientLocInit    = true;
		SetActorLocation(Loc, false, nullptr, ETeleportType::None);
		return;
	}

	if (ClientInterpAlpha < 1.0f && ClientInterpSpeed > KINDA_SMALL_NUMBER)
	{
		const float SegDist = FVector::Dist(ClientPrevLoc, ClientTargetLoc);

		if (SegDist > KINDA_SMALL_NUMBER)
		{
			// Alpha 증가량 = 이동 속도 * DeltaTime / 구간 거리
			// = (SegDist/0.05) * DeltaTime / SegDist
			// = DeltaTime / 0.05
			// → 항상 0.05초에 걸쳐 선형 이동, 속도 변화 없음
			ClientInterpAlpha += DeltaTime / 0.05f;
		}

		ClientInterpAlpha = FMath::Min(ClientInterpAlpha, 1.0f);
		ClientSmoothLoc   = FMath::Lerp(ClientPrevLoc, ClientTargetLoc, ClientInterpAlpha);
	}
	else
	{
		// 이미 목표 도달 또는 OnRep 지연 중
		// 현재 위치 유지 (다음 OnRep이 올 때까지)
		ClientSmoothLoc = ClientTargetLoc;
	}

	SetActorLocation(ClientSmoothLoc, false, nullptr, ETeleportType::None);
}

void AGarbageEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AGarbageEnemyBase, RepLocation, COND_None, REPNOTIFY_Always);
}