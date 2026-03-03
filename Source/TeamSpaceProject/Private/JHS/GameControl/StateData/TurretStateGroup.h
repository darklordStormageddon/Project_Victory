// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "TurretStateGroup.generated.h"

class AJHSGameState;
class ATurretStand;
class ATurretChair;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTurretStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTurretStateGroup();

private:
	const int32 HUNDRED = 100;

	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY()
	TMap<int32, FTurretData> _turretDataMap;

	UPROPERTY()
	TMap<E_AMMO_TYPE, FAmmoData> _ammoDataMap;

	UPROPERTY()
	TMap<E_AMMO_TYPE, TObjectPtr<ATurretStand>> _turretStandMap;

	UPROPERTY(Replicated)
	E_AMMO_TYPE _mainTurretType = E_AMMO_TYPE::NONE;

	UPROPERTY(ReplicatedUsing = "OnRep_TurretDataArray")
	TArray<FTurretData> _replicatedTurretDataArray;

	UPROPERTY(ReplicatedUsing = "OnRep_AmmoDataArray")
	TArray<FAmmoData> _replicatedAmmoDataArray;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_TurretDataArray();

	UFUNCTION()
	void OnRep_AmmoDataArray();

	const int32 CONSUME_AMMO = -1;

	bool _isInfiniteMagMode = false;

	UPROPERTY();
	TObjectPtr<ATurretChair> _turretChair = nullptr;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeTurretState(TObjectPtr<AJHSGameState> GameState);

	void UpdateTurretState();

	TArray<FPurchaseData*> GetTurretPurchaseDataArray();

private:
	TArray<FPurchaseData*> GetTurretPurchaseDataArray(bool IsMainTurret, E_AMMO_TYPE AmmoType);

public:
	TArray<FPurchaseData*> GetAmmoPurchaseDataArray();

private:
	void TryPurchaseTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 FieldIndex);

	void TryPurchaseAmmo(E_AMMO_TYPE AmmoType, int32 FieldIndex);

public:
	void SetStartSettings(bool IsInfiniteMagMode, E_AMMO_TYPE MainTurretType, bool IsStartEquipMainTurret, TArray<E_AMMO_TYPE> _startEquipAutoTurretArray);

	bool TryGetTurretFireInterval(bool IsMainTurret, E_AMMO_TYPE AmmoType, float* OutFireCoolTime);

	bool TryFireTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType);

	bool TryReloadTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType);

private:
	void TryEquipTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType);

	UFUNCTION(Server, Reliable)
	void ServerEquipTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEquipTurret(bool IsMainTurret, E_AMMO_TYPE AmmoType);

private:
	void LoadTurretDataTable();

	int32 GetTurretKey(bool IsMainTurret, E_AMMO_TYPE AmmoType);

	bool TryGetTurretData(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData*& OutTurretData);

public:
	bool TryGetAmmoData(E_AMMO_TYPE AmmoType, FAmmoData*& OutAmmoData);

private:
	bool TryGetTurretStand(bool IsMainTurret, E_AMMO_TYPE AmmoType, TObjectPtr<ATurretStand>& OutTurretStand);

	void ChangeTurretAmmo(bool IsMainTurret, E_AMMO_TYPE AmmoType, int32 ChangeValue);

	void ExecuteTurretEvent(bool IsMainTurret, E_AMMO_TYPE AmmoType, FTurretData TurretData);

	void SyncTurretStateToReplicated();
};
