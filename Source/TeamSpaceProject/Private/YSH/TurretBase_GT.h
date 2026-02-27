// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "PSJ/TaskPawnBase.h"
#include "TurretBase_GT.generated.h"



class APSJ_Character;
class ATaskChair;

class USceneComponent;
class UStaticMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AProjectile;
class AJHSGameState;

UCLASS()
class TEAMSPACEPROJECT_API ATurretBase_GT : public ATaskPawnBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATurretBase_GT();

	// [추가] 하차 입력을 위한 액션 (우주선과 동일한 키 사용 권장)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//// [추가] 연결된 조종석 (내릴 때 정보 갱신용)
	//UPROPERTY(VisibleInstanceOnly, Category = "Connection")
	//ATaskChair* LinkedSeat;
	
public:

	// [신규] 탑승 처리 함수 (서버 호출)
	void SetPilot(APSJ_Character* NewPilot, ATaskChair* Chair);

	// 오버라이드하게  수정
	virtual void Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot) override;


	// 오버라이드하게  수정
	virtual void DisembarkCharacter() override;

	// [수정 후] Implementation만 오버라이드 합니다.
	virtual void Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector ExitLoc, FRotator ExitRot) override;

	//// [신규] 하차 입력 바인딩 함수
	//void Input_Exit(const FInputActionValue& Value);



	void AddYawInput(float YawInputDegPerSec, float DeltaTime);
	void AddPitchInput(float PitchInputDegPerSec, float DeltaTime);

	// 터렛 포지션 설정 (Main, Left, Right 중 하나)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Position")
	E_TURRET_POSITION TurretPosition = E_TURRET_POSITION::Main;

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

	//머즐 컴포넌트
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MainMuzzle = nullptr;

	// 카메라 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera = nullptr;

	//IMC,InputAction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> TurretMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction = nullptr;

	// IMC 우선순위
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 MappingPriority = 1;

	UPROPERTY(EditAnywhere, Category = "Turret|Limit")
	float MinPitch = -20.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Limit")
	float MaxPitch = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Control")
	float TurretYawSpeed = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Control")
	float TurretPitchSpeed = 30.0f;

	// 포신 지연 추적 설정 (무게감 표현)
	UPROPERTY(EditAnywhere, Category = "Turret|Control", meta = (ClampMin = "0.1", ClampMax = "20.0"))
	float BarrelFollowSpeed = 3.0f;  // 포신이 카메라를 따라가는 속도 (낮을수록 더 느리고 무거운 느낌)

	UPROPERTY(EditAnywhere, Category = "Turret|Control")
	bool bEnableBarrelLag = true;  // 포신 지연 효과 활성화

	// 카메라 추적 설정
	UPROPERTY(EditAnywhere, Category = "Turret|Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CameraPitchFollowRatio = 1.0f;  // 1.0 = 완전 추적, 0.5 = 절반만, 0.0 = 추적 안함

	UPROPERTY(EditAnywhere, Category = "Turret|Camera")
	bool bSmoothCameraFollow = false;

	UPROPERTY(EditAnywhere, Category = "Turret|Camera", meta = (EditCondition = "bSmoothCameraFollow"))
	float CameraPitchFollowSpeed = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Turret|Camera", meta = (EditCondition = "bSmoothCameraFollow"))
	float CameraYawFollowSpeed = 15.0f;

	// 발사 속도 배율 (업그레이드용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Fire", meta = (AllowPrivateAccess = "true"))
	float FireRateMultiplier = 1.0f;
	// 발사체 속도 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Fire", meta = (AllowPrivateAccess = "true"))
	float ProjectileSpeedMultiplier = 1.0f;
	// 데미지 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Fire", meta = (AllowPrivateAccess = "true"))
	float DamageMultiplier = 1.0f;
	// 발사체 클래스
	UPROPERTY(EditAnywhere, Category = "Turret|Fire")
	TSubclassOf<AProjectile> ProjectileClass;
	// 파티클 시스템 (Cascade)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Effects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> MuzzleFlashEffect = nullptr;
	// 발사 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Effects", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> FireSound = nullptr;
	// 카메라 쉐이크
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Effects", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> FireCameraShake = nullptr;
	// 이펙트 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Effects", meta = (AllowPrivateAccess = "true"))
	FRotator MuzzleFlashRotationOffset = FRotator(-90.0f, 0.0f, 0.0f);
	// 이펙트 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Effects", meta = (AllowPrivateAccess = "true"))
	FVector MuzzleFlashLocationOffset = FVector::ZeroVector;
	// 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Effects", meta = (AllowPrivateAccess = "true"))
	float MuzzleFlashScale = 1.0f;

	//내부 변수
	float TargetYaw = 0.0f;
	float TargetPitchRoll = 0.0f;
	float CurrentCameraYaw = 0.0f;
	float InitialSpringArmRoll = 0.0f;

	// 카메라의 포탑 대비 Yaw 오프셋
	float CameraYawOffset = 0.0f;

	// 발사 관련 내부 변수
	bool bIsLeftMuzzleNext = true;
	bool bIsFiring = false;
	float TimeSinceLastFire = 0.0f;

	// GameState 캐싱
	TObjectPtr<AJHSGameState> _cachedGameState = nullptr;

	// Enhanced Input 콜백 함수
	void Look(const FInputActionValue& Value);
	void Fire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);
	void TryFire();
};