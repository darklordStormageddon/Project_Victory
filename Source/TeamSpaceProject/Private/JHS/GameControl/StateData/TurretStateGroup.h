// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "TurretStateGroup.generated.h"

class AJHSGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UTurretStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTurretStateGroup();

private:
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	FTurretData _turretData;

	const int32 CONSUME_AMMO = -1;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeTurretState(TObjectPtr<AJHSGameState> GameState, FTurretData InitTurretData);

	void UpdateTurretState();

	float GetTurretFireCoolTime() { return _turretData.FireCoolTime; }

	bool TryFireTurret();

	void ReloadTurret();

private:
	void ChangeTurretAmmo(int32 ChangeValue);
};
