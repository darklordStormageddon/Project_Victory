// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/EnemyBase.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/SpaceManager.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"

#include "CJH/Enemy/Manager/EnemyManagerComponent.h"
#include "CJH/Enemy/Manager/GarbageEnemyManagerComponent.h"
#include "CJH/Enemy/Manager/SpawnedEnemyManagerComponent.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	FireComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireComp"));
	/*SpaceObjectComp = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));*/
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	bReplicates = true;
	SetReplicateMovement(false);
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	HealthComp -> OnDeath.AddDynamic(this, &AEnemyBase::EnemyDeath);
}

void AEnemyBase::SetInfo()
{
	if (!HasAuthority())
		return;

	// 적 크기에 비례하여 능력치 증감
	SetActorScale3D(this->NewScale);

	_targetInfo.Max_HP *= this->_targetInfo.Size;
	_targetInfo.Attack_Damage *= this->_targetInfo.Size;

	HealthComp->SetCurrentHP(_targetInfo.Max_HP);
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

	HealthComp->CurrentHealth -= 20.f;

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
bool AEnemyBase::DistanceCheck(float _condition)
{
	if (!HasAuthority())
		return false;

	if (!IsValid(_spaceShip))
		return false;

	float Distance = FVector::Dist(GetActorLocation(), _spaceShip->GetActorLocation());

	if (Distance <= _condition)
		return true;

	return false;
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

	Murdered = true;

	if (DeathParticle)
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
				HealthComp->CurrentHealth = HealthComp->MaxHealth;
			},
			RiviveTime,
			false);
	}
	else
		this -> Destroy();
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!HasAuthority())
		return;

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearAllTimersForObject(this);

	UGarbageEnemyManagerComponent* GarbageComponent = Cast<UGarbageEnemyManagerComponent>(EnemyComponent);
	USpawnedEnemyManagerComponent* SpawnedComponent = Cast<USpawnedEnemyManagerComponent>(EnemyComponent);

	if (GarbageComponent)
		GarbageComponent->RemoveEnemies(this);
	else if (SpawnedComponent)
		SpawnedComponent->RemoveEnemies(this);

	USpaceManager* OutSpaceManager = nullptr;

	if (!UStaticFunctionLibrary::TryGetSpaceManager(OutSpaceManager))
	{
		UE_LOG(LogTemp, Error, TEXT("AEnemyBase: OutSpaceManager is nullptr"));
		return;
	}

	OutSpaceManager->RemoveSpaceObject(SpaceObjectComp);

	Super::EndPlay(EndPlayReason);

	if (EnemyGarbage && Murdered)
		SpawnGarbageSetting();
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