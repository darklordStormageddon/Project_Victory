// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"
#include "InteractableContainerUI.generated.h"

class UUIBase;

/**
 * 
 */
UCLASS()
class AInteractableContainerUI : public AInteractableActorBase
{
	GENERATED_BODY()
	
protected:
	void OnInteractEnter(TObjectPtr<UUIBase> OpenedUI) override;

	void OnInteractExit(TObjectPtr<UUIBase> ClosedUI) override;
};
