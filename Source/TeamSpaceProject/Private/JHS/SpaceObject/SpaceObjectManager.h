// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceObjectManager.generated.h"

class ASpaceObjectBase;

UENUM(BlueprintType)
enum class E_PUZZLE_TRIGGER_TYPE : uint8
{
	SpaceShip UMETA(DisplayName = "SpaceShip"),				// 快林急
	SpaceGarbage UMETA(DisplayName = "SpaceGarbage"),		// 快林 企扁拱
	Asteroid = 0 UMETA(DisplayName = "Asteroid"),			// 家青己
	Enemy UMETA(DisplayName = "Enemy"),						// 利

	SIZE UMETA(DisplayName = "SIZE")
};

USTRUCT(BlueprintType)
struct FSpaceObjectData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ASpaceObjectBase> SpaceObjectPtr;

	UPROPERTY()
	E_PUZZLE_TRIGGER_TYPE SpaceObjectType;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotator;
};

UCLASS()
class ASpaceObjectManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceObjectManager();

private:
	TMap<TObjectPtr<ASpaceObjectBase>, FSpaceObjectData> _spaceObjectMap;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceObjectManager|SpaceObject")
	TArray<TObjectPtr<ASpaceObjectBase>> _initializeSpaceObjectArray;

public:
	TMap<TObjectPtr<ASpaceObjectBase>, FSpaceObjectData> GetSpaceObjectMap() { return _spaceObjectMap; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void UpdateSpaceObject(FSpaceObjectData SpaceObjectData);

	void RemoveSpaceObject(TObjectPtr<ASpaceObjectBase> NewSpaceObjectPtr);
};
