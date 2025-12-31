// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "SpaceObjectComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USpaceObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpaceObjectComponent();

private:
	TObjectPtr<USpaceObjectManager> _spaceObjectManager = nullptr;

	TObjectPtr<AActor> _owner = nullptr;

	FTimerHandle _updateTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceObject|Update")
	E_SPACE_OBJECT_TYPE _spaceObjctType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceObject|Update")
	float _updateInterval = 0.1f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void InitializeSpaceObject();

	void Send();

	void UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData);

	UFUNCTION(Server, Reliable)
	void Server_UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData);

	FSpaceObjectData GetSpaceObjectData();
};
