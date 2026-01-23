// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "TurretBase_AT2.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class AProjectile;
class AJHSGameState;

UCLASS()
class ATurretBase_AT2 : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATurretBase_AT2();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void AddYawInput(float YawInputDegPerSec, float DeltaTime);
	void AddPitchInput(float PitchInputDegPerSec, float DeltaTime);

	// 터렛 포지션 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Position")
	E_TURRET_POSITION TurretPosition = E_TURRET_POSITION::Right;

private:
	// === 컴포넌트 ===
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> YawPivot = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> PitchPivot = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BarrelMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MainMuzzle = nullptr;

	// === 자동 타겟팅 설정 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float DetectionRange = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Auto")
	float MinPitch = -20.0f;

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
	FRotator MuzzleFlashRotationOffset = FRotator(-90.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	float MuzzleFlashScale = 1.0f;

	// === 데미지 설정 ===
	UPROPERTY(EditAnywhere, Category = "Turret|Fire|Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Fire|Damage")
	bool bShowDamageDebug = false;

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

	// 반구형 감지 체크 헬퍼 함수
	bool IsTargetInHemisphere(AActor* Target) const;
};