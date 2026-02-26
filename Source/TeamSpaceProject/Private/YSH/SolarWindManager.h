// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SolarWindManager.generated.h"

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSolarWindWarning, float, WarningDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSolarWindImpact);

UCLASS()
class TEAMSPACEPROJECT_API ASolarWindManager : public AActor
{
	GENERATED_BODY()

public:
	ASolarWindManager();

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Solar Wind")
	FOnSolarWindWarning OnSolarWindWarning;

	UPROPERTY(BlueprintAssignable, Category = "Solar Wind")
	FOnSolarWindImpact OnSolarWindImpact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Timing")
	float InitialDelay = 180.0f; // 게임 시작 후 첫 이벤트까지 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Timing")
	float CooldownDuration = 180.0f; // 이벤트 후 금지 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Timing")
	float MinRandomInterval = 60.0f; // 최소 랜덤 간격

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Timing")
	float MaxRandomInterval = 300.0f; // 최대 랜덤 간격

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Warning")
	float WarningDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Camera")
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Camera")
	float ShakeIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Camera")
	float ShakeDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Audio")
	class USoundCue* WarningSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Audio")
	class USoundCue* ImpactSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Debug")
	bool bShowDebugInfo = true;

	// 수동 트리거 함수들
	UFUNCTION(BlueprintCallable, Category = "Solar Wind")
	void TriggerSolarWindManually();

	UFUNCTION(BlueprintCallable, Category = "Solar Wind")
	void TestCameraShakeImmediately();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

private:
	// 타이머 핸들
	FTimerHandle SolarWindIntervalTimerHandle;
	FTimerHandle WarningTimerHandle;
	FTimerHandle ShakeTimerHandle;

	// 경보 상태
	bool bIsWarningActive;
	float CurrentWarningTime;

	// 카메라 셰이크 추적
	TArray<TWeakObjectPtr<class UCameraShakeBase>> ActiveCameraShakes;

	// 내부 함수들
	void StartSolarWindEvent();
	void TriggerSolarWindWarning();
	void TriggerSolarWindImpact();
	void ApplyCameraShake();
	void StopCameraShake();
	void CleanupActiveCameraShakes();

	// 다음 이벤트 스케줄링
	void ScheduleNextEvent();

	// 첫 이벤트 여부 추적
	bool bIsFirstEvent = true;
};