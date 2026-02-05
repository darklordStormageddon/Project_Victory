#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Manager/EnemyManagerComponent.h"

#include "CJH/Enemy/Base/EnemyBase.h"

#include "GarbageEnemyManagerComponent.generated.h"

class AJHSGameMode;
class ASpaceStation;
class AEnemyBase;
class AGarbageEnemyBase;

USTRUCT()
struct FDroneOrbitData
{
	GENERATED_BODY()

	FDroneOrbitData()
	{
		OrbitAxis = FVector::UpVector;
		OrbitRadius = 1500.0f;
		Phase = 0.0f;
		AngularSpeed = 30.0f;
	}

	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	FVector OrbitAxis;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float OrbitRadius;

	// 현재 그룹 위상(도). RotateSpeed에 따라 증가
	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float Phase;

	// 개별 공전 회전 속도 범위
	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float AngularSpeed;

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UGarbageEnemyManagerComponent : public UEnemyManagerComponent
{
	GENERATED_BODY()

protected:

	TArray<AGarbageEnemyBase*> GarbageEnemies;

	// 적 정보 맵
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	TArray<TSubclassOf<AGarbageEnemyBase>> _Enemy;

	// 공전 반경 범위(컴포넌트 시작 시 랜덤으로 선택되어 모든 자식이 공유)
	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float OrbitMinDistance = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float OrbitMaxDistance = 1800.0f;

	// 실제 사용되는 공전 반경(BeginPlay에서 결정)
	float OrbitDistance = 1500.0f;

	// 공전 평면의 피치 범위 (0~45): 컴포넌트 시작 시 랜덤으로 ±범위 내에서 선택되어 모든 자식이 공유
	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float OrbitPitchRange = 45.0f;

	// 실제 사용되는 피치값(도, BeginPlay에서 결정, 모든 자식 공유)
	float OrbitPitch = 0.0f;

	// 궤도 평면을 기울일 축(컴포넌트 시작 시 랜덤으로 결정, 수평 벡터)
	FVector OrbitTiltAxis = FVector::RightVector;

	// (선택) 개별 공전 회전 속도 범위
	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float RotateMinSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float RotateMaxSpeed = 60.0f;

	float RotateSpeed;

	// 현재 그룹 위상(도). RotateSpeed에 따라 증가
	float CurrentOrbitPhase = 0.0f;

	// 매 스폰된 Junior(AGarbageEnemyBase)와 그들이 따라갈 목표 월드 좌표
	TMap<AGarbageEnemyBase*, FVector> JuniorEnemies;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float _minSpawn = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float _maxSpawn = 5.0f;

	// 디버그 플래그/값
	bool candebug = true;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool _debug = true;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool _debugOrbitLine = true;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool _debugOrbitPoint = true;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	float _debugRadius = 100.0f;

	FQuat TiltQuat;
	float AngleStep;

	int Num;

	// 타이머 핸들
	FTimerHandle OrbitTimerHandle;

	// 개별 드론 궤도 데이터
	TMap<AGarbageEnemyBase*, FDroneOrbitData> OrbitData;

protected:
	// Sets default values for this component's properties
	UGarbageEnemyManagerComponent();

	void GarbageSpawnSetting();

	void OrbitSet();

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SpawnInMap(FVector SpawnLocation, FRotator SpawnRotation, TArray<TSubclassOf<AGarbageEnemyBase>> _spawn_enemy);
	void SpawnEnemy(TSubclassOf<AGarbageEnemyBase> Enemy, FVector SpawnLocation, FRotator SpawnRotator);


	// 드론(또는 Junior) 속성 설정 (spawn 이후)
	void SetDroneProperties(AEnemyBase* Drone);

	// Garbage 중심을 기준으로 Junior의 목표 위치 계산 및 할당
	void BuildOrbitStructure();
	void TurnOrbit();  // DeltaTime 파라미터 없음

	// 개별 궤도 초기화
	void InitializeOrbitData(AGarbageEnemyBase* Drone);

	// Garbage 중심 좌표 얻기(Owner 기준)
	FVector GetCenterLocation()
	{
			if (_owner)
				return _owner->GetActorLocation();
			if (GetOwner())
				return GetOwner()->GetActorLocation();

			return
				FVector::ZeroVector;
	};

	// 디버그 그리기
	void DebugVector();

	void CanDebug() {
		candebug = true;
	}

public:
	virtual void RemoveEnemies(AEnemyBase* _removeEnemy) override;
	virtual void DeleteAllEnemy() override;
};
