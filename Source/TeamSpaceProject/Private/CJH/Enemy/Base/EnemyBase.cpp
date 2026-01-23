// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Enemy/Base/EnemyBase.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "CJH/Enemy/Manager/EnemyManagerComponent.h"
#include "CJH/Enemy/Manager/GarbageEnemyManagerComponent.h"
#include "CJH/Enemy/Manager/SpawnedEnemyManagerComponent.h"

#include "KSM/HealthComponent.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	FireComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireComp"));
	SpaceObjectComp = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	HealthComp -> OnDeath.AddDynamic(this, &AEnemyBase::EnemyDeath);
}

void AEnemyBase::SetInfo()
{
	// 적 크기에 비례하여 능력치 증감
	SetActorScale3D(this->NewScale);

	_spawnedInfo.Max_HP *= this->Size;
	_spawnedInfo.Attack_Damage *= this->Size;
	_spawnedInfo.Value *= this->Size;

	HealthComp->SetCurrentHP(_spawnedInfo.Max_HP);
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(MinusDebug)
		if (DelayBool)
			MinusHp();

}

void AEnemyBase::MinusHp()
{
	DelayBool = false;

	FTimerHandle MinusHandle;

	HealthComp->CurrentHealth -= 20.f;

	GetWorld()->GetTimerManager().SetTimer(MinusHandle, [this]() {DelayBool = true; }, 1.0f, false);
}

bool AEnemyBase::TargetHPCheck()
{
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
	if (!IsValid(_spaceShip) && _spaceShip)
		return false;

	float Distance = FVector::Dist(GetActorLocation(), _spaceShip->GetActorLocation());

	if (Distance <= _condition)
		return true;

	return false;
}

void AEnemyBase::SetTargetShip(TSubclassOf<AActor> Targetenemy) 
{ 
	_spaceShip = UGameplayStatics::GetActorOfClass(GetWorld(), Targetenemy);
}

void AEnemyBase::EnemyDeath()
{
	Murdered = true;

	if (DeathParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DeathParticle,
			GetActorTransform()
		);
	}
	
	this -> Destroy();
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearAllTimersForObject(this);

	UGarbageEnemyManagerComponent* GarbageComponent = Cast<UGarbageEnemyManagerComponent>(EnemyComponent);
	USpawnedEnemyManagerComponent* SpawnedComponent = Cast<USpawnedEnemyManagerComponent>(EnemyComponent);

	if (GarbageComponent)
		GarbageComponent->RemoveEnemies(this);
	else if (SpawnedComponent)
		SpawnedComponent->RemoveEnemies(this);

	SpaceObject_Remove();

	Super::EndPlay(EndPlayReason);

	if(EnemyGarbage && Murdered)
		GetWorld()->SpawnActor<AActor>(EnemyGarbage, this->GetActorTransform());
}

void AEnemyBase::SpaceObject_Remove()
{
	USpaceObjectManager* _spaceManager = nullptr;

	if (UStaticFunctionLibrary::TryGetSpaceObjectManager(_spaceManager))
	{
		_spaceManager->RemoveSpaceObject(SpaceObjectComp);
	}
}
