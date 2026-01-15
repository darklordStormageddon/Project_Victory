// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/SpawnedEnemyBase.h"

#include "Components/ArrowComponent.h"

#include "Turret.generated.h"

class ABullet;

UCLASS()
class ATurret : public ASpawnedEnemyBase
{
private:
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Base")
	UArrowComponent* MuzzleArrow1;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	UArrowComponent* MuzzleArrow2;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	UStaticMeshComponent* TurretMesh;

	FVector MuzzleLocation1;
	FVector MuzzleLocation2;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TSubclassOf<ABullet> Bullet;

	bool CanFire = true;
	FVector MoveDirection;
	FRotator RotationDirection;

	float RotateSpeed = 50.0f;
private:
	void Move(float DeltaTime);
	void Fire();
	void EnableFiring();
	void LookTarget();
	void SetDirection();

protected:
	ATurret();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
