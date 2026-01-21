// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 충돌 이벤트
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

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

	// === 적중 이펙트 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	UParticleSystem* HitEffect;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	FVector HitEffectScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, Category = "Projectile|Effects")
	float HitEffectLifetime = 2.0f;

	// === 디버그 ===
	UPROPERTY(EditAnywhere, Category = "Projectile|Debug")
	bool bShowDebugHit = false;

private:
	void SpawnHitEffect(const FVector& HitLocation, const FRotator& HitRotation);
};