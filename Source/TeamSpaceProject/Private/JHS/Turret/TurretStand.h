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

	UPROPERTY(EditAnywhere, Category = "TurretStand")
	E_AMMO_TYPE _standType = E_AMMO_TYPE::NONE;

	UPROPERTY()
	E_AMMO_TYPE _turretAmmoType = E_AMMO_TYPE::NONE;

public:
	E_AMMO_TYPE GetStandType() { return _standType; }

	E_AMMO_TYPE GetTurretType() { return _turretAmmoType; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeTurretStand(TObjectPtr<UTurretStateGroup> TurretStateGroup);

	void SetTurretType(E_AMMO_TYPE TurretAmmoType);

	bool CanEquipTurret() const;

	bool TryEquipTurret(TObjectPtr<AActor> Turret);

private:
	void RemoveCurrentTurret();
};
