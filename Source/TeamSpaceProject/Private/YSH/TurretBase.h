// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TurretBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class TEAMSPACEPROJECT_API ATurretBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATurretBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void AddYawInput(float YawInputDegPerSec, float DeltaTime);
	void AddPitchInput(float PitchInputDegPerSec, float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root = nullptr;

	// 좌우 회전(Yaw) 축
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> YawPivot = nullptr;

	// 상하 회전(Pitch) 축 - Roll에서 Pitch로 변경
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> PitchPivot = nullptr;

	//외형
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BarrelMesh = nullptr;

	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera = nullptr;

private:
	UPROPERTY(EditAnywhere, Category = "Turret|Limit")
	float MinPitch = -20.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Limit")
	float MaxPitch = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Control")
	float TurretYawSpeed = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Control")
	float TurretPitchSpeed = 30.0f;

	// 입력 처리 함수
	void LookYaw(float Value);
	void LookPitch(float Value);
};