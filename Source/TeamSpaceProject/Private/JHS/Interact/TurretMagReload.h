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
	TObjectPtr<UTurretStateGroup> _turretStateGroup = nullptr;

	TObjectPtr<UUIPanelTurretMagReload> _uiTurretMagReload = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|TurretMagReload")
	E_TURRET_POSITION _turretPosition;

protected:
	virtual void BeginPlay() override;

	void OnInteractEnter(TObjectPtr<UUIBase> OpenedUI) override;

	void OnInteractExit(TObjectPtr<UUIBase> ClosedUI) override;
};
