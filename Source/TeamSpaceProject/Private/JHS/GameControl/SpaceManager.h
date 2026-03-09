// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SpaceManager.generated.h"

class ASpaceStation;
class USpaceObjectComponent;
class ADriveSeatRader;

UENUM(BlueprintType)
enum class E_SPACE_OBJECT_TYPE : uint8
{
	SpaceStation = 0 UMETA(DisplayName = "SpaceStation"),		// 快林 沥芭厘
	SpaceGarbage UMETA(DisplayName = "SpaceGarbage"),			// 快林 企扁拱
	Asteroid UMETA(DisplayName = "Asteroid"),					// 家青己
	Enemy UMETA(DisplayName = "Enemy"),							// 利

	SpaceShip UMETA(DisplayName = "SpaceShip"),					// 快林急
};

USTRUCT(BlueprintType)
struct FSpaceObjectData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USpaceObjectComponent> SpaceObjectComponent = nullptr;

	UPROPERTY()
	E_SPACE_OBJECT_TYPE SpaceObjectType = E_SPACE_OBJECT_TYPE::SpaceStation;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotator = FRotator::ZeroRotator;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class USpaceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USpaceManager();

private:
	UPROPERTY()
	TObjectPtr<ASpaceStation> _spaceStation = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> _spaceShip = nullptr;

	UPROPERTY()
	TMap<USpaceObjectComponent*, FSpaceObjectData> _spaceObjectMap;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Space Manager|Debug")
	bool _isDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Space Manager|Space")
	float _spaceRadius = 100000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Space Manager|Rader")
	float _driveRaderRadius = 10000.0f;

public:
	UFUNCTION()
	float GetSpaceRadius() { return _spaceRadius; }

public:
	UFUNCTION()
	TMap<USpaceObjectComponent*, FSpaceObjectData> GetSpaceObjectMap() { return _spaceObjectMap; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Space Station")
	void SetSpaceShip(AActor* SpaceShip) { _spaceShip = SpaceShip; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category = "GameMode|Space Station")
	ASpaceStation* GetSpaceStation();

public:
	void InitializeDriveRader(TObjectPtr<ADriveSeatRader> DriveRader, TObjectPtr<AActor> SpaceShip);

	void UpdateSpaceObject(FSpaceObjectData SpaceObjectData);

	void RemoveSpaceObject(TObjectPtr<USpaceObjectComponent> NewSpaceObjectPtr);
};
