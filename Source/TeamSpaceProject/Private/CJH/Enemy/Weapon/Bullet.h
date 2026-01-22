// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

#include "Bullet.generated.h"


UCLASS()
class ABullet : public AActor
{
	GENERATED_BODY()

private:
	AActor* _owner;

	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	float Speed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	float BulletLifeTime = 5.0f;

	FVector Direction;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	USphereComponent* Collision;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
	UParticleSystem* HitParticle;

private:
	void MoveToTarget(float DeltaTime);

public:	
	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBulletOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetTarget(FVector _TargetDirection) { Direction = _TargetDirection; }
	void SetOwner(AActor* _getOwner) { _owner = _getOwner; }
	void SetDamage(float _getDamage) { Damage = _getDamage; }
};
