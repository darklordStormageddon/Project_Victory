// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "SpaceObjectBase.generated.h"

UCLASS()
class ASpaceObjectBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceObjectBase();

private:
	TObjectPtr<USpaceObjectManager> _spaceObjectManager = nullptr;

	FTimerHandle _updateTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceObject|Update")
	E_SPACE_OBJECT_TYPE _spaceObjctType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceObject|Update")
	float _updateInterval = 0.1f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void MovementTick(float DeltaTime) {};

public:
	void InitializeSpaceObject();

private:
	void UpdateMovement(float DeltaTime);

	void UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData);

	UFUNCTION(Server, Reliable)
	void Server_UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData);

	void Send();

	FSpaceObjectData GetSpaceObjectData();
};
