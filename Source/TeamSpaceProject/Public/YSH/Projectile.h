// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class TEAMSPACEPROJECT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent = nullptr;

	// 발사체 메시
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;

	// 발사체 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	// 발사체 설정
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float InitialSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float LifeSpan = 5.0f;

	// 충돌 처리
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};