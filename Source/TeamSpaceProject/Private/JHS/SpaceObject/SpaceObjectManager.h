// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceObjectManager.generated.h"

class USpaceObjectComponent;
class ASpaceStation;

UENUM(BlueprintType)
enum class E_SPACE_OBJECT_TYPE : uint8
{
	SpaceStation = 0 UMETA(DisplayName = "SpaceStation"),		// 우주 정거장
	SpaceGarbage UMETA(DisplayName = "SpaceGarbage"),			// 우주 폐기물
	Asteroid UMETA(DisplayName = "Asteroid"),					// 소행성
	Enemy UMETA(DisplayName = "Enemy"),							// 적

	SpaceShip UMETA(DisplayName = "SpaceShip"),					// 우주선
};

USTRUCT(BlueprintType)
struct FSpaceObjectData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USpaceObjectComponent> SpaceObjectComponent;

	UPROPERTY()
	E_SPACE_OBJECT_TYPE SpaceObjectType = E_SPACE_OBJECT_TYPE::SpaceStation;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotator = FRotator::ZeroRotator;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USpaceObjectManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpaceObjectManager();

private:
	UPROPERTY()
	TMap<USpaceObjectComponent*, FSpaceObjectData> _spaceObjectMap;

	UPROPERTY()
	TObjectPtr<ASpaceStation> _spaceStation = nullptr;

public:
	UFUNCTION()
	TMap<USpaceObjectComponent*, FSpaceObjectData> GetSpaceObjectMap() { return _spaceObjectMap; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void InitializeSpaceObjectManager();

public:
	void UpdateSpaceObject(FSpaceObjectData SpaceObjectData);

	void RemoveSpaceObject(TObjectPtr<USpaceObjectComponent> NewSpaceObjectPtr);
};
