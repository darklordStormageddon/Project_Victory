// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "JHSGameMode.generated.h"

class USpaceObjectManager;

/**
 * 
 */
UCLASS()
class AJHSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AJHSGameMode();

protected:
	virtual void BeginPlay() override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Space Object Manager")
	TObjectPtr<USpaceObjectManager> _spaceObjectManager;

public:
	UFUNCTION()
	USpaceObjectManager* GetSpaceObjectManager() { return _spaceObjectManager; }
};
