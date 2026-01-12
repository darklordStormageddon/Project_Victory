// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableChairBase.h"
#include "InteractableChairExample.generated.h"

/**
 * 
 */
UCLASS()
class AInteractableChairExample : public AInteractableChairBase
{
	GENERATED_BODY()



protected:
	void OnInteractEnter() override;
	
	void OnInteractExit() override;
};
