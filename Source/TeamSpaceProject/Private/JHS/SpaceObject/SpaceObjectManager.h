// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceObjectManager.generated.h"

class ASpaceObjectBase;

UENUM(BlueprintType)
enum class E_SPACE_OBJECT_TYPE : uint8
{
	SpaceShip = 0 UMETA(DisplayName = "SpaceShip"),				// 우주선
	SpaceGarbage UMETA(DisplayName = "SpaceGarbage"),			// 우주 폐기물
	Asteroid UMETA(DisplayName = "Asteroid"),					// 소행성
	Enemy UMETA(DisplayName = "Enemy"),							// 적
};

USTRUCT(BlueprintType)
struct FSpaceObjectData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ASpaceObjectBase> SpaceObjectPtr;

	UPROPERTY()
	E_SPACE_OBJECT_TYPE SpaceObjectType;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotator;
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
	TMap<ASpaceObjectBase*, FSpaceObjectData> _spaceObjectMap;

public:
	UFUNCTION()
	TMap<ASpaceObjectBase*, FSpaceObjectData> GetSpaceObjectMap() { return _spaceObjectMap; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void UpdateSpaceObject(FSpaceObjectData SpaceObjectData);

	void RemoveSpaceObject(TObjectPtr<ASpaceObjectBase> NewSpaceObjectPtr);
};
