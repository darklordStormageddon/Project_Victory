// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"
#include "InteractableChairExample.generated.h"

/**
 * 
 */
UCLASS()
class AInteractableChairExample : public AInteractableActorBase
{
	GENERATED_BODY()



protected:
	void OnInteractEnter(AActor* Caller, TObjectPtr<UUIBase> OpenedUI) override;

	void OnInteractExit(AActor* Caller, TObjectPtr<UUIBase> ClosedUI) override;
};
