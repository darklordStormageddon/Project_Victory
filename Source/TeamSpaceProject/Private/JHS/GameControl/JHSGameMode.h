// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "JHS/Player/SpaceStation.h"
#include "JHSGameMode.generated.h"

class USpaceObjectManager;

UCLASS()
class AJHSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AJHSGameMode();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Space Station")
	TObjectPtr<ASpaceStation> _spaceStation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Space Object Manager")
	TObjectPtr<USpaceObjectManager> _spaceObjectManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Space Size")
	float _spaceRadius = 1000.0f;

public:
	UFUNCTION()
	float GetSpaceRadius() { return _spaceRadius; }

protected:
	virtual void BeginPlay() override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
	UFUNCTION()
	ASpaceStation* GetSpaceStation();

	UFUNCTION()
	USpaceObjectManager* GetSpaceObjectManager() { return _spaceObjectManager; }
};
