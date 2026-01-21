// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "TurretStand.generated.h"

class UTurretStateGroup;

UCLASS()
class ATurretStand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATurretStand();

private:
	UPROPERTY()
	TObjectPtr<UTurretStateGroup> _turretManager = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> _turret = nullptr;

	UPROPERTY()
	E_AMMO_TYPE _ammoType;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TurretStand")
	E_TURRET_POSITION _turretPosition;

public:
	E_TURRET_POSITION GetTurretPosition() { return _turretPosition; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeTurretStand(TObjectPtr<UTurretStateGroup> TurretStateGroup);

	bool TryEquipTurret(TObjectPtr<AActor> Turret, E_AMMO_TYPE AmmoType);

	E_AMMO_TYPE GetAmmoType();
};
