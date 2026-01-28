// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/TargetBase.h"

#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

#include "Asteroid.generated.h"

class UAsteroidComponent;
class USpaceObjectComponent;

UCLASS()

class AAsteroid : public ATargetBase
{
	GENERATED_BODY()
private:
	float MoveDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float MinSize;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	float MaxSize;

	UPROPERTY(VisibleAnywhere, Category = "Collision")
	USphereComponent* Collision;
public:
	UPROPERTY()
	UAsteroidComponent* AsteroidComponent;

	FVector TargetLocation;

private:
	UPROPERTY(EditDefaultsOnly, Category = "SpaceObject")
	USpaceObjectComponent* SpaceObjectComp;

	FVector Direction;

	FRotator ConstRotaion;

	float RotateSpeed;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float MinRotateSpeed = 5.0f; 
	UPROPERTY(EditAnywhere, Category = "Rotation")
	float MaxRotateSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	float ShockConstant = 0.01f;//충격량 보정 상수

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
	UParticleSystem* HitParticle;

protected:
	UFUNCTION()
	void OnDestroy();

private:
	void MoveAsteroid(float DeltaTime);
	void SetAsteroidRot();

	bool CalculateInterceptPoint(
		const FVector& AsteroidPos,
		const FVector& ShipPos,
		const FVector& ShipVelocity
	);

public:
	// Sets default values for this actor's properties
	AAsteroid();

	float DestroyDistance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpaceObject_Remove();
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DebugDrawing();

	void SetAsteroidInfo(
		const FTargetInfo& InAsteroidInfo,
		FVector VSpaceShip,
		FVector Velocity);

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);
};

