// Fill out your copyright notice in the Description page of Project Settings.

#include "CJH/Manager/SpawnSequenceComponent.h"

#include "CJH/Asteroid/StaticAsternoidManagerComponent.h"
#include "CJH/Stability/ASManagerComponent.h"
#include "CJH/Enemy/Manager/EnemyRoundManagerComponent.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"

USpawnSequenceComponent::USpawnSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpawnSequenceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	StaticAsteroidManager = GetOwner()->FindComponentByClass<UStaticAsternoidManagerComponent>();
	SatelliteManager = GetOwner()->FindComponentByClass<UASManagerComponent>();
	EnemyManager = GetOwner()->FindComponentByClass<UEnemyManagerComponent>();

	// 1→2 체인: 스태틱 소행성 완료 → 인공위성 시작
	if (StaticAsteroidManager)
	{
		StaticAsteroidManager->OnStaticAsteroidSpawnComplete.AddDynamic(this, &USpawnSequenceComponent::OnStaticAsteroidComplete);
	}

	// 2→3 체인: 인공위성 전부 완료 → 드론 시작
	if (SatelliteManager)
	{
		SatelliteManager->OnAllSatellitesSpawnComplete.AddDynamic(this, &USpawnSequenceComponent::OnAllSatellitesComplete);
	}

	// 이벤트 매니저 수신
	UEventManager* EventManager = nullptr;
	if (UStaticFunctionLibrary::TryGetEventManager(EventManager) && EventManager)
	{
		OnStartStageHandle = EventManager->AddListener<UEventOnStartStage>(
			[this](UEventOnStartStage* Event)
			{
				if (!Event)
					return;

				const int32 Stage = Event->Stage <= 0 ? AutoStartStage : Event->Stage;
				StartSequence(Stage);
			}
		);

		OnEndStageHandle = EventManager->AddListener<UEventOnEndStage>(
			[this](UEventOnEndStage* Event)
			{
				if (!Event)
					return;

				EndSequence();
			}
		);
	}

	if (bAutoStart)
		StartSequence(AutoStartStage);
}

void USpawnSequenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UEventManager* EventManager = nullptr;
		if (UStaticFunctionLibrary::TryGetEventManager(EventManager) && EventManager)
		{
			if (OnStartStageHandle.IsValid())
			{
				EventManager->DelListener<UEventOnStartStage>(OnStartStageHandle);
				OnStartStageHandle.Reset();
			}

			if (OnEndStageHandle.IsValid())
			{
				EventManager->DelListener<UEventOnEndStage>(OnEndStageHandle);
				OnEndStageHandle.Reset();
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USpawnSequenceComponent::StartSequence(int32 Round)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	CurrentRound = Round;

	// 1단계: 스태틱 소행성 스폰
	if (StaticAsteroidManager)
	{
		StaticAsteroidManager->StartRound();
	}
	else
	{
		// 없으면 2단계로
		OnStaticAsteroidComplete();
	}
}

void USpawnSequenceComponent::EndSequence()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (EnemyManager)
		EnemyManager->EndRound();

	if (StaticAsteroidManager)
		StaticAsteroidManager->EndRound();

	CurrentRound = 0;
}

void USpawnSequenceComponent::OnStaticAsteroidComplete()
{
	// 2단계: 인공위성 스폰
	if (SatelliteManager)
	{
		SatelliteManager->StartSpawn();
	}
	else
	{
		// 없으면 3단계로
		OnAllSatellitesComplete();
	}
}

void USpawnSequenceComponent::OnAllSatellitesComplete()
{
	// 3단계: 드론 스폰
	if (EnemyManager)
	{
		EnemyManager->StartRound(CurrentRound);
	}
}
