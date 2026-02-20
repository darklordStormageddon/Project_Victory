#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Manager/EnemySpawnComponent.h"

#include "CJH/Enemy/Base/EnemyBase.h"

#include "GarbageEnemySpawnComponent.generated.h"

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

	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float Phase;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float AngularSpeed;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UGarbageEnemySpawnComponent : public UEnemySpawnComponent
{
	GENERATED_BODY()

protected:
	TArray<AGarbageEnemyBase*> GarbageEnemies;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float OrbitMinDistance = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float OrbitMaxDistance = 1800.0f;

	float OrbitDistance = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float OrbitPitchRange = 45.0f;

	float OrbitPitch = 0.0f;

	FVector OrbitTiltAxis = FVector::RightVector;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float RotateMinSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	float RotateMaxSpeed = 60.0f;

	float RotateSpeed = 0.0f;

	float CurrentOrbitPhase = 0.0f;

	TMap<AGarbageEnemyBase*, FVector> JuniorEnemies;

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
	float AngleStep = 0.0f;

	FTimerHandle OrbitTimerHandle;

	TMap<AGarbageEnemyBase*, FDroneOrbitData> OrbitData;

protected:
	UGarbageEnemySpawnComponent();

	void OrbitSet();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetDroneProperties(AEnemyBase* Drone);
	void BuildOrbitStructure();
	void TurnOrbit();
	void InitializeOrbitData(AGarbageEnemyBase* Drone);

	virtual void OnEnemySpawned(AEnemyBase* SpawnedEnemy) override;

	FVector GetCenterLocation()
	{
		if (_owner)
			return _owner->GetActorLocation();
		if (GetOwner())
			return GetOwner()->GetActorLocation();

		return FVector::ZeroVector;
	};

	void DebugVector();

	void CanDebug() { candebug = true; }

public:
	virtual void RemoveEnemies(AEnemyBase* _removeEnemy) override;
	virtual void DeleteAllEnemy() override;
};
