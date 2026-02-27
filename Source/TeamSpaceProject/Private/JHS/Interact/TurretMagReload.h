// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"

#include "TurretMagReload.generated.h"

class UTurretStateGroup;
class UUIPanelTurretMagReload;

UCLASS()
class ATurretMagReload : public AInteractableActorBase
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<UTurretStateGroup> _turretStateGroup = nullptr;

	UPROPERTY()
	TObjectPtr<UUIPanelTurretMagReload> _uiTurretMagReload = nullptr;

protected:
	virtual void BeginPlay() override;

	void OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI) override;

	void OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI) override;
};
