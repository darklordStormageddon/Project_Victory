// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/SolarWindManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include "JHS/GameControl/StageChangeExample.h" 
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/GameControl/JHSGameMode.h"

ASolarWindManager::ASolarWindManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsWarningActive = false;
	CurrentWarningTime = 0.0f;
	bIsFirstEvent = true;
}

void ASolarWindManager::BeginPlay()
{
	Super::BeginPlay();

	if (!UStaticFunctionLibrary::TryGetEventManager(EventManager) || !EventManager)
		return;

	OnStartStageHandle = EventManager->AddListener<UEventOnStartStage>(
		[this](UEventOnStartStage* Event)
		{
			HandleStartStage(Event);
		}
	);

	OnEndStageHandle = EventManager->AddListener<UEventOnEndStage>(
		[this](UEventOnEndStage* Event)
		{
			HandleEndStage(Event);
		}
	);
}

void ASolarWindManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SolarWindIntervalTimerHandle);
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);

	CleanupActiveCameraShakes();

	if (EventManager)
	{
		if (OnStartStageHandle.IsValid())
		{
			EventManager->DelListener<UEventOnStartStage>(OnStartStageHandle);
		}
		if (OnEndStageHandle.IsValid())
		{
			EventManager->DelListener<UEventOnEndStage>(OnEndStageHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ASolarWindManager::HandleStartStage(UEventOnStartStage* Event)
{
	if (!Event)
		return;

	bIsStageActive = true;
	bIsFirstEvent = true;

	if (bShowDebugInfo)
	{

	}

	if (InitialDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			SolarWindIntervalTimerHandle,
			this,
			&ASolarWindManager::StartSolarWindEvent,
			InitialDelay,
			false
		);

		if (bShowDebugInfo)
		{

		}
	}
}

void ASolarWindManager::HandleEndStage(UEventOnEndStage* Event)
{
	if (!Event)
		return;

	bIsStageActive = false;

	if (bShowDebugInfo)
	{
	}


	GetWorldTimerManager().ClearTimer(SolarWindIntervalTimerHandle);
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);


	bIsWarningActive = false;
	CleanupActiveCameraShakes();
}

void ASolarWindManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (!bIsStageActive)
		return;


	if (bIsWarningActive)
	{
		CurrentWarningTime -= DeltaTime;

		if (bShowDebugInfo)
		{
			DrawDebugString(
				GetWorld(),
				FVector(0, 0, 200),
				FString::Printf(TEXT("⚠ SOLAR WIND WARNING: %.1f seconds"), FMath::Max(0.0f, CurrentWarningTime)),
				nullptr,
				FColor::Red,
				0.0f,
				true,
				2.0f
			);
		}
	}
}


void ASolarWindManager::StartSolarWindEvent()
{

	if (!bIsStageActive)
		return;


	if (bShowDebugInfo)
	{

	}

	TriggerSolarWindWarning();

	GetWorldTimerManager().SetTimer(
		WarningTimerHandle,
		this,
		&ASolarWindManager::TriggerSolarWindImpact,
		WarningDuration,
		false
	);

}

void ASolarWindManager::TriggerSolarWindWarning()
{

	if (!bIsStageActive)
		return;


	bIsWarningActive = true;
	CurrentWarningTime = WarningDuration;

	OnSolarWindWarning.Broadcast(WarningDuration);
}

void ASolarWindManager::TriggerSolarWindImpact()
{

	if (!bIsStageActive)
		return;


	bIsWarningActive = false;

	OnSolarWindImpact.Broadcast();

	ApplyCameraShake();

	ScheduleNextEvent();
}

void ASolarWindManager::ScheduleNextEvent()
{

	if (!bIsStageActive)
		return;

	float NextInterval = 0.0f;

	if (bIsFirstEvent)
	{

		NextInterval = CooldownDuration;
		bIsFirstEvent = false;


	}
	else
	{

		float RandomDelay = FMath::RandRange(MinRandomInterval, MaxRandomInterval);
		NextInterval = CooldownDuration + RandomDelay;


	}


	GetWorldTimerManager().SetTimer(
		SolarWindIntervalTimerHandle,
		this,
		&ASolarWindManager::StartSolarWindEvent,
		NextInterval,
		false 
	);

	if (bShowDebugInfo)
	{

	}
}

void ASolarWindManager::ApplyCameraShake()
{


	if (!CameraShakeClass)
	{

		return;
	}

	CleanupActiveCameraShakes();

	UWorld* World = GetWorld();
	if (!World)
	{

		return;
	}


	int32 PlayerControllerCount = 0;
	int32 SuccessfulShakeCount = 0;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		PlayerControllerCount++;
		APlayerController* PC = It->Get();



		if (PC)
		{
			if (PC->PlayerCameraManager)
			{


				UCameraShakeBase* ShakeInstance = PC->PlayerCameraManager->StartCameraShake(
					CameraShakeClass,
					ShakeIntensity
				);

				if (ShakeInstance)
				{
					SuccessfulShakeCount++;
					ActiveCameraShakes.Add(ShakeInstance);

				}
				else
				{

				}
			}
			else
			{

			}
		}
	}



	if (SuccessfulShakeCount > 0)
	{

	}
	else
	{

	}


	if (ShakeDuration > 0.0f)
	{

		GetWorldTimerManager().SetTimer(
			ShakeTimerHandle,
			this,
			&ASolarWindManager::StopCameraShake,
			ShakeDuration,
			false
		);
	}


}

void ASolarWindManager::StopCameraShake()
{
	CleanupActiveCameraShakes();

	if (bShowDebugInfo)
	{

	}
}

void ASolarWindManager::CleanupActiveCameraShakes()
{
	UWorld* World = GetWorld();
	if (!World)
		return;


	for (TWeakObjectPtr<UCameraShakeBase> ShakeInstance : ActiveCameraShakes)
	{
		if (ShakeInstance.IsValid())
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				if (PC && PC->PlayerCameraManager)
				{
					PC->PlayerCameraManager->StopCameraShake(
						ShakeInstance.Get(),
						false 
					);
				}
			}
		}
	}


	ActiveCameraShakes.Empty();
}

void ASolarWindManager::TriggerSolarWindManually()
{


	if (bShowDebugInfo)
	{

	}

	StartSolarWindEvent();
}

void ASolarWindManager::TestCameraShakeImmediately()
{


	ApplyCameraShake();
}