// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"

#include "InteractableStageController.generated.h"

UCLASS()
class AInteractableStageController : public AInteractableActorBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, Category = "StageController")
	bool _isStartStage = false;

protected:
	void OnInteractEnter(AActor* Caller, TObjectPtr<UUIBase> OpenedUI) override;

	void OnInteractExit(AActor* Caller, TObjectPtr<UUIBase> ClosedUI) override;

private:
	void InteractController(AActor* Caller);
};
