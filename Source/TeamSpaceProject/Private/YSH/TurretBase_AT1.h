// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "TurretBase_AT1.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AProjectile;
class AJHSGameState;

UCLASS()
class ATurretBase_AT1 : public APawn
{
	GENERATED_BODY()

public:
	ATurretBase_AT1();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void AddYawInput(float YawInputDegPerSec, float DeltaTime);
	void AddPitchInput(float PitchInputDegPerSec, float DeltaTime);

	// 터렛 포지션 설정 (Main, Left, Right 중 하나)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Position")
	E_TURRET_POSITION TurretPosition = E_TURRET_POSITION::Left;

private:
	// === 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* YawPivot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* PitchPivot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* BarrelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* MainMuzzle;

	// === 자동 타겟팅 설정 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float DetectionRange = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float MinPitch = -30.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float MaxPitch = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	TSubclassOf<AActor> TargetActorClass;

	// === 반구형 감지 설정 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	bool bUseHemisphericalDetection = true;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float MinimumTargetHeightOffset = 0.0f;

	AActor* CurrentTarget = nullptr;

	// === 디버그 설정 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Debug")
	bool bShowDebugRange = true;

	UPROPERTY(EditAnywhere, Category = "Turret|Debug")
	bool bShowDebugLineOfSight = true;

	UPROPERTY(EditAnywhere, Category = "Turret|Debug")
	FColor DebugRangeColor = FColor::Yellow;

	UPROPERTY(EditAnywhere, Category = "Turret|Debug")
	FColor DebugTargetFoundColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "Turret|Debug")
	FColor DebugLineOfSightColor = FColor::Red;

	// === 발사 관련 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	UParticleSystem* MuzzleFlashEffect;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	FVector MuzzleFlashLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	FRotator MuzzleFlashRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	float MuzzleFlashScale = 1.0f;

	// === 탄 퍼짐(Spread) 설정 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Fire|Spread", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float SpreadConeAngle = 2.0f;  // 원뿔의 각도 (도 단위), 0이면 정확한 조준

	UPROPERTY(EditAnywhere, Category = "Turret|Fire|Spread")
	bool bEnableSpread = true;  // 탄 퍼짐 활성화 여부

	UPROPERTY(EditAnywhere, Category = "Turret|Fire|Spread")
	bool bShowSpreadDebug = false;  // 탄 퍼짐 디버그 시각화

	float TimeSinceLastFire = 0.0f;
	bool bIsLeftMuzzleNext = true;

	// === GameState 캐싱 ===
	TObjectPtr<AJHSGameState> _cachedGameState = nullptr;

	// === 내부 함수 ===
	void FindAndTrackTarget(float DeltaTime);
	void RotateTowardsTarget(float DeltaTime);
	void TryAutoFire();
	bool IsTargetInRange() const;
	bool IsTargetInLineOfSight() const;
	void DrawDebugVisualization();

	// 랜덤 스프레드 계산 헬퍼 함수
	FRotator GetSpreadRotation(const FRotator& BaseRotation) const;

	// 반구형 감지 체크 헬퍼 함수
	bool IsTargetInHemisphere(AActor* Target) const;
};