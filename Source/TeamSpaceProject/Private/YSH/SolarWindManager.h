// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SolarWindManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSolarWindWarning, float, TimeUntilImpact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSolarWindImpact);

/**
 * 태양풍 월드 이벤트를 관리하는 클래스
 */
UCLASS()
class TEAMSPACEPROJECT_API ASolarWindManager : public AActor
{
	GENERATED_BODY()

public:
	ASolarWindManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========== 태양풍 설정 ==========

	/** 즉시 카메라 셰이크 테스트 (경고 없이 바로 실행) */
	UFUNCTION(BlueprintCallable, Category = "Solar Wind|Debug")
	void TestCameraShakeImmediately();

	/** 태양풍 발생 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Settings")
	float SolarWindInterval = 180.0f; // 3분마다

	/** 경보 후 실제 충격까지의 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Settings")
	float WarningDuration = 10.0f;

	/** 카메라 셰이크 지속 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Settings")
	float ShakeDuration = 3.0f;

	/** 카메라 셰이크 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Settings")
	float ShakeIntensity = 1.0f;

	// ========== 이벤트 델리게이트 ==========

	/** 경보 발생 시 브로드캐스트 (UI에서 구독) */
	UPROPERTY(BlueprintAssignable, Category = "Solar Wind|Events")
	FOnSolarWindWarning OnSolarWindWarning;

	/** 태양풍 충격 발생 시 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "Solar Wind|Events")
	FOnSolarWindImpact OnSolarWindImpact;

	// ========== 카메라 셰이크 ==========

	/** 카메라 셰이크 클래스 (블루프린트에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Effects")
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	/** 경보 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Effects")
	class USoundBase* WarningSoundCue;

	/** 충격 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Effects")
	class USoundBase* ImpactSoundCue;

	// ========== 디버그 ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solar Wind|Debug")
	bool bShowDebugInfo = true;

	/** 수동으로 태양풍 트리거 (테스트용) */
	UFUNCTION(BlueprintCallable, Category = "Solar Wind")
	void TriggerSolarWindManually();

private:
	// 타이머 핸들
	FTimerHandle SolarWindIntervalTimerHandle;
	FTimerHandle WarningTimerHandle;
	FTimerHandle ShakeTimerHandle;

	// 현재 경보 남은 시간
	float CurrentWarningTime;
	bool bIsWarningActive;

	// 활성화된 카메라 셰이크 인스턴스 저장
	TArray<TWeakObjectPtr<class UCameraShakeBase>> ActiveCameraShakes;

	// 태양풍 이벤트 함수
	void StartSolarWindEvent();
	void TriggerSolarWindWarning();
	void TriggerSolarWindImpact();
	void ApplyCameraShake();
	void StopCameraShake();
	void CleanupActiveCameraShakes();
};