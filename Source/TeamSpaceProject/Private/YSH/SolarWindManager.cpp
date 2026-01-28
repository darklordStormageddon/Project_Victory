// Fill out your copyright notice in the Description page of Project Settings.

#include "YSH/SolarWindManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ASolarWindManager::ASolarWindManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsWarningActive = false;
	CurrentWarningTime = 0.0f;
}

void ASolarWindManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("🎬 SolarWindManager BeginPlay() CALLED"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	// 첫 태양풍 이벤트 스케줄링
	if (SolarWindInterval > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("⏰ Setting up timer with interval: %.1f seconds"), SolarWindInterval);

		GetWorldTimerManager().SetTimer(
			SolarWindIntervalTimerHandle,
			this,
			&ASolarWindManager::StartSolarWindEvent,
			SolarWindInterval,
			true // 반복
		);

		UE_LOG(LogTemp, Warning, TEXT("✅ Timer set successfully. First event in %.1f seconds"), SolarWindInterval);

		if (bShowDebugInfo)
		{
			UE_LOG(LogTemp, Warning, TEXT("✓ Solar Wind Manager initialized. First event in %.1f seconds"), SolarWindInterval);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ SolarWindInterval is <= 0! Timer NOT set! Value: %.1f"), SolarWindInterval);
	}
}

void ASolarWindManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 타이머 정리
	GetWorldTimerManager().ClearTimer(SolarWindIntervalTimerHandle);
	GetWorldTimerManager().ClearTimer(WarningTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);

	// 실행 중인 카메라 셰이크 정리
	CleanupActiveCameraShakes();

	Super::EndPlay(EndPlayReason);
}

void ASolarWindManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 경보 중일 때 카운트다운 표시
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
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("🌞 StartSolarWindEvent() CALLED"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("🌞 Solar Wind Event Started!"));
	}

	// 경보 시작
	TriggerSolarWindWarning();

	// 경보 시간 후 충격 발생 스케줄링
	UE_LOG(LogTemp, Warning, TEXT("⏰ Scheduling impact in %.1f seconds"), WarningDuration);

	GetWorldTimerManager().SetTimer(
		WarningTimerHandle,
		this,
		&ASolarWindManager::TriggerSolarWindImpact,
		WarningDuration,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("✅ Impact timer set"));
}

void ASolarWindManager::TriggerSolarWindWarning()
{
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("⚠ TriggerSolarWindWarning() CALLED"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	bIsWarningActive = true;
	CurrentWarningTime = WarningDuration;

	// 경보 이벤트 브로드캐스트 (UI가 이를 받아서 경보 메시지 표시)
	UE_LOG(LogTemp, Warning, TEXT("📡 Broadcasting warning event..."));
	OnSolarWindWarning.Broadcast(WarningDuration);

	// 경보 사운드 재생
	if (WarningSoundCue)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔊 Playing warning sound"));
		UGameplayStatics::PlaySound2D(this, WarningSoundCue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠ No warning sound set"));
	}

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠ Solar Wind Warning! Impact in %.1f seconds"), WarningDuration);
	}
}

void ASolarWindManager::TriggerSolarWindImpact()
{
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("💥 TriggerSolarWindImpact() CALLED"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	bIsWarningActive = false;

	// 충격 이벤트 브로드캐스트
	UE_LOG(LogTemp, Warning, TEXT("📡 Broadcasting impact event..."));
	OnSolarWindImpact.Broadcast();

	// 충격 사운드 재생
	if (ImpactSoundCue)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔊 Playing impact sound"));
		UGameplayStatics::PlaySound2D(this, ImpactSoundCue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠ No impact sound set"));
	}

	// 카메라 셰이크 적용
	UE_LOG(LogTemp, Error, TEXT("🎥 About to call ApplyCameraShake()..."));
	ApplyCameraShake();
	UE_LOG(LogTemp, Error, TEXT("🎥 ApplyCameraShake() returned"));

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("💥 Solar Wind Impact!"));
	}
}

void ASolarWindManager::ApplyCameraShake()
{
	UE_LOG(LogTemp, Warning, TEXT("====== ApplyCameraShake() Called ======"));

	if (!CameraShakeClass)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Camera Shake Class not set in Solar Wind Manager!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ Camera Shake Class is set: %s"), *CameraShakeClass->GetName());

	// 이전 셰이크 정리
	CleanupActiveCameraShakes();

	// 모든 플레이어 컨트롤러에 카메라 셰이크 적용
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ World is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ World found: %s"), *World->GetName());

	int32 PlayerControllerCount = 0;
	int32 SuccessfulShakeCount = 0;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		PlayerControllerCount++;
		APlayerController* PC = It->Get();

		UE_LOG(LogTemp, Warning, TEXT("  [%d] Found PlayerController: %s"), PlayerControllerCount, PC ? *PC->GetName() : TEXT("NULL"));

		if (PC)
		{
			if (PC->PlayerCameraManager)
			{
				UE_LOG(LogTemp, Warning, TEXT("  ✅ PlayerCameraManager found for %s"), *PC->GetName());
				UE_LOG(LogTemp, Warning, TEXT("  Attempting to start camera shake with Intensity: %.2f"), ShakeIntensity);

				UCameraShakeBase* ShakeInstance = PC->PlayerCameraManager->StartCameraShake(
					CameraShakeClass,
					ShakeIntensity
				);

				if (ShakeInstance)
				{
					SuccessfulShakeCount++;
					ActiveCameraShakes.Add(ShakeInstance);
					UE_LOG(LogTemp, Warning, TEXT("  ✅✅ Camera Shake Instance Created: %s"), *ShakeInstance->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("  ❌ StartCameraShake returned NULL!"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("  ❌ PlayerCameraManager is NULL for %s"), *PC->GetName());
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("📊 Summary: %d Player Controllers found, %d Shake Instances created"),
		PlayerControllerCount, SuccessfulShakeCount);

	if (SuccessfulShakeCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("📹 Camera Shake applied to %d players (Intensity: %.2f)"),
			SuccessfulShakeCount, ShakeIntensity);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ NO CAMERA SHAKES WERE APPLIED!"));
	}

	// ShakeDuration 후 셰이크 중지
	if (ShakeDuration > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("⏰ Scheduling shake stop in %.2f seconds"), ShakeDuration);
		GetWorldTimerManager().SetTimer(
			ShakeTimerHandle,
			this,
			&ASolarWindManager::StopCameraShake,
			ShakeDuration,
			false
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("====== ApplyCameraShake() End ======\n"));
}

void ASolarWindManager::StopCameraShake()
{
	CleanupActiveCameraShakes();

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("📹 Camera Shake stopped"));
	}
}

void ASolarWindManager::CleanupActiveCameraShakes()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 저장된 모든 카메라 셰이크 인스턴스 중지
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
						false // 즉시 중지
					);
				}
			}
		}
	}

	// 배열 초기화
	ActiveCameraShakes.Empty();
}

void ASolarWindManager::TriggerSolarWindManually()
{
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("🔧 MANUAL SOLAR WIND TRIGGER CALLED"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug Info is ON"));
	}

	StartSolarWindEvent();
}

void ASolarWindManager::TestCameraShakeImmediately()
{
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("🧪 IMMEDIATE CAMERA SHAKE TEST"));
	UE_LOG(LogTemp, Error, TEXT("========================================"));

	ApplyCameraShake();
}