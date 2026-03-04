// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Enemy/Base/EnemyBase.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/SpaceManager.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"	

#include "CJH/Enemy/Manager/EnemySpawnComponent.h"
#include "CJH/Enemy/Manager/GarbageEnemySpawnComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 20.0f;
	MinNetUpdateFrequency = 10.0f;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	if (HealthComp)
	{
		FTimerHandle InitHandle;

		GetWorld()->GetTimerManager().SetTimer(InitHandle, [this]()
		{
			HealthComp->OnDeath.AddDynamic(this, &AEnemyBase::EnemyDeath);
		}, 0.01f, false);
	}
}

void AEnemyBase::SetInfo()
{
	if (!HasAuthority())
		return;

	if(bShowStatsDebug)
		DebugShowStat();

	// 적 크기에 비례하여 능력치 증감
	SetActorScale3D(this->NewScale);

	_targetInfo.Max_HP *= this->_targetInfo.Size;
	_targetInfo.Attack_Damage *= this->_targetInfo.Size;

	// ===== 체력 업데이트 (바인딩 후) =====
	if (HealthComp)
	{
		HealthComp->MaxHealth = _targetInfo.Max_HP;
		HealthComp->CurrentHealth = _targetInfo.Max_HP;
	}
}

void AEnemyBase::DebugShowStat()
{
	UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::DebugShowStat - Size: %f, Max_HP: %f, Attack_Damage: %f, Speed: %f, Attack_Speed: %f, Attack_Range: %f, Detection_Range: %f"),
		_targetInfo.Size,
		_targetInfo.Max_HP,
		_targetInfo.Attack_Damage,
		_spawnedInfo.Attack_Speed,
		_spawnedInfo.Attack_Range,
		_spawnedInfo.Detection_Range,
		_targetInfo.Speed
	);
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
		return;

	if(MinusDebug)
		if (DelayBool)
			MinusHp();
}

void AEnemyBase::MinusHp()
{
	if (!HasAuthority())
		return;

	DelayBool = false;

	FTimerHandle MinusHandle;

	if (HealthComp)
	{
		HealthComp->CurrentHealth -= 20.f;
	}

	GetWorld()->GetTimerManager().SetTimer(MinusHandle, [this]() {DelayBool = true; }, 1.0f, false);
}

bool AEnemyBase::TargetHPCheck()
{
	if (!HasAuthority())
		return false;

	if (!Target)
		return false;

	if (UHealthComponent* Health = this->Target->FindComponentByClass<UHealthComponent>())
	{
		if (Health->CurrentHealth <= 0)
			return false;

		return true;
	}
	return false;
}

// 플레이어와의 거리 체크
bool AEnemyBase::DistanceCheck(float Condition)
{
	if (!HasAuthority() || !IsValid(_spaceShip))
		return false;

	const float DistSq = FVector::DistSquared(
		GetActorLocation(),
		_spaceShip->GetActorLocation()
	);

	return DistSq <= Condition * Condition;
}

void AEnemyBase::SetTargetShip(TSubclassOf<AActor> Targetenemy) 
{ 
	if (!HasAuthority())
		return;

	_spaceShip = UGameplayStatics::GetActorOfClass(GetWorld(), Targetenemy);
}

void AEnemyBase::EnemyDeath()
{
	if (!HasAuthority())
		return;

	// ===== 안전 체크: 이미 죽었으면 리턴 =====
	if (Murdered)
		return;

	Murdered = true;

	// ===== 파티클 안전하게 스폰 (현재 위치에서만) =====
	if (DeathParticle && GetActorLocation().Length() > 100.f)  // 0,0,0 확인
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DeathParticle,
			GetActorTransform()
		);
	}

	if (DebugRevive)
	{
		FTimerHandle DebugReviveHandle;
		GetWorld()->GetTimerManager().SetTimer(
			DebugReviveHandle, 
			[this](){
				if (HealthComp && Murdered)
				{
					Murdered = false;
					HealthComp->CurrentHealth = HealthComp->MaxHealth;
					UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::EnemyDeath - Revived!"));
				}
			},
			RiviveTime,
			false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (IsValid(this))
			{
				Destroy();
			}
		});
	}
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!HasAuthority())
		return;

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearAllTimersForObject(this);

	if (IsValid(EnemyComponent))
	{
		UGarbageEnemySpawnComponent* GarbageComponent = Cast<UGarbageEnemySpawnComponent>(EnemyComponent);
		UEnemySpawnComponent* SpawnComponent = Cast<UEnemySpawnComponent>(EnemyComponent);

		if (GarbageComponent)
			GarbageComponent->RemoveEnemies(this);
		else if (SpawnComponent)
			SpawnComponent->RemoveEnemies(this);
	}

	USpaceManager* OutSpaceManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetSpaceManager(this, OutSpaceManager))
	{
		UE_LOG(LogTemp, Error, TEXT("AEnemyBase: OutSpaceManager is nullptr"));
		return;
	}

	OutSpaceManager->RemoveSpaceObject(SpaceObjectComp);

	Super::EndPlay(EndPlayReason);

	// ===== 죽었을 때만 폐기물 스폰 =====
	if (Murdered)
	{
		SpawnGarbageSetting();
	}
}

void AEnemyBase::SetEnemyInfo(
	const FTargetInfo& InEnemyInfo,
	const FEnemyInfo& InSpawnedInfo)
{
	if (!HasAuthority())
		return;

	_targetInfo = InEnemyInfo;
	_spawnedInfo = InSpawnedInfo;

	SetActorScale3D(FVector(_targetInfo.Size));
}