// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "KSMGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AKSMGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	virtual void PostLogin(APlayerController* NewPlayer) override;
};