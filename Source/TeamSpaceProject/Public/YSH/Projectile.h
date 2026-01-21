// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UHealthComponent;

UCLASS()
class AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// 충돌 이벤트
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	// 타겟 설정 함수 (외부에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetHomingTarget(AActor* Target);

protected:
	// === 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// === 투사체 설정 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Settings")
	float InitialSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Settings")
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Settings")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Settings")
	float LifeSpan = 5.0f;

	// === 지연 가속 설정 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Acceleration", meta = (ClampMin = "0.0"))
	bool bEnableDelayedAcceleration = false;

	UPROPERTY(EditAnywhere, Category = "Projectile|Acceleration", meta = (EditCondition = "bEnableDelayedAcceleration", ClampMin = "0.0"))
	float LaunchSpeed = 500.0f;  // 발사 직후의 느린 속도

	UPROPERTY(EditAnywhere, Category = "Projectile|Acceleration", meta = (EditCondition = "bEnableDelayedAcceleration", ClampMin = "0.0"))
	float BoostSpeed = 8000.0f;  // 급가속 후 도달할 속도

	UPROPERTY(EditAnywhere, Category = "Projectile|Acceleration", meta = (EditCondition = "bEnableDelayedAcceleration", ClampMin = "0.0"))
	float AccelerationDelay = 0.3f;  // 가속 시작까지의 지연 시간 (초)

	UPROPERTY(EditAnywhere, Category = "Projectile|Acceleration", meta = (EditCondition = "bEnableDelayedAcceleration", ClampMin = "0.0"))
	float AccelerationRate = 10000.0f;  // 가속도 (단위/초²)

	// === 지연 유도 설정 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Homing")
	bool bEnableDelayedHoming = false;  // 지연 유도 활성화

	UPROPERTY(EditAnywhere, Category = "Projectile|Homing", meta = (EditCondition = "bEnableDelayedHoming", ClampMin = "0.0"))
	float HomingDelay = 0.5f;  // 유도 시작까지의 지연 시간 (초)

	UPROPERTY(EditAnywhere, Category = "Projectile|Homing", meta = (EditCondition = "bEnableDelayedHoming", ClampMin = "0.0"))
	float HomingAcceleration = 8000.0f;  // 유도 가속도

	UPROPERTY(EditAnywhere, Category = "Projectile|Homing", meta = (EditCondition = "bEnableDelayedHoming", ClampMin = "0.0"))
	float HomingTurnSpeed = 360.0f;  // 유도 회전 속도 (도/초) - 높을수록 급격하게 휨

	UPROPERTY(EditAnywhere, Category = "Projectile|Homing", meta = (EditCondition = "bEnableDelayedHoming"))
	bool bUseHomingAcceleration = true;  // false면 기본 ProjectileMovementComponent의 Homing 사용

	// === 적중 이펙트 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	UParticleSystem* HitEffect;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	UNiagaraSystem* HitEffectNiagara;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	FVector HitEffectScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	float HitEffectLifetime = 2.0f;

	// === 가속 이펙트 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	UNiagaraSystem* BoostEffectNiagara;  

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	USoundBase* BoostSound;

	// 부스트 이펙트 위치/회전/스케일 오프셋
	UPROPERTY(EditAnywhere, Category = "Projectile|Effects|Boost")
	FVector BoostEffectLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects|Boost")
	FRotator BoostEffectRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects|Boost")
	FVector BoostEffectScale = FVector(1.0f);

	// === 디버그 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Debug")
	bool bShowDebugHit = false;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug")
	bool bShowDebugAcceleration = false;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug")
	bool bShowDebugHoming = false;

private:
	void SpawnHitEffect(const FVector& HitLocation, const FRotator& HitRotation);
	void UpdateAcceleration(float DeltaTime);
	void UpdateHoming(float DeltaTime);
	void CleanupBoostEffect();

	// 내부 상태 변수
	float TimeAlive = 0.0f;
	bool bHasBoosted = false;
	bool bHomingActivated = false;

	// 타겟 참조
	UPROPERTY()
	AActor* HomingTarget = nullptr;

	// 부스트 이펙트 컴포넌트 참조
	UPROPERTY()
	UNiagaraComponent* ActiveBoostEffect = nullptr;
};