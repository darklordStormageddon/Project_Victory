// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaticAsternoidManagerComponent.generated.h"

class ASpaceStation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaticAsteroidSpawnComplete);

USTRUCT(BlueprintType)
struct FStaticAsteroidInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Asteroid|Class")
	TSubclassOf<AActor> StaticAsteroidClass;

	UPROPERTY(EditAnywhere, Category = "Asteroid|Scale")
	float MinSize = 5.f;
	UPROPERTY(EditAnywhere, Category = "Asteroid|Scale")
	float MaxSize = 20.f;
};

// 소행성 하나의 스폰 정보 - 복제용
USTRUCT()
struct FStaticAsteroidSpawnData
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AActor> AsteroidClass;

	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	float Scale = 1.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UStaticAsternoidManagerComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	AActor* SpaceStation = nullptr;

	// 서버: 실제 스폰된 액터 목록
	UPROPERTY()
	TArray<AActor*> SpawnedAsteroids;

	// 클라이언트: 로컬 스폰된 액터 목록
	UPROPERTY()
	TArray<AActor*> ClientSpawnedAsteroids;

	// 복제되는 스폰 데이터 목록 - OnRep으로 클라이언트에서 스폰
	UPROPERTY(ReplicatedUsing = OnRep_SpawnDataList)
	TArray<FStaticAsteroidSpawnData> RepSpawnDataList;

	UPROPERTY(EditAnywhere, Category = "Asteroid")
	TArray<FStaticAsteroidInfo> AsteroidInfoArray;

	UPROPERTY(EditAnywhere, Category = "Radius")
	float SpaceStationSaveRadius = 5000.f;
	UPROPERTY(EditAnywhere, Category = "Radius")
	float StaticAsteroidSaveRadius = 500.f;

	float SpawnRadius = 0.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 SpawnMinNum = 20;
	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 SpawnMaxNum = 40;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 MaxRetries = 100;

	FDelegateHandle OnStartStageHandle;
	FDelegateHandle OnEndStageHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Round")
	bool bAutoStart = false;

private:
	void ClearRoundActors();
	void InitSpaceRadius();

	// 클라이언트에서 RepSpawnDataList를 받아 실제로 스폰
	UFUNCTION()
	void OnRep_SpawnDataList();

	void SpawnAsteroidOnClient(const FStaticAsteroidSpawnData& Data);

public:	
	UStaticAsternoidManagerComponent();

	UPROPERTY(BlueprintAssignable, Category = "Asteroid")
	FOnStaticAsteroidSpawnComplete OnStaticAsteroidSpawnComplete;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SpawnStaticAsteroid();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void StartRound();

	UFUNCTION(BlueprintCallable)
	void EndRound();
};
