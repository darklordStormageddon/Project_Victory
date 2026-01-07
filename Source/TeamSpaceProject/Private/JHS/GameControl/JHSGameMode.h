// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "JHS/Player/SpaceStation.h"
#include "JHSGameMode.generated.h"

class USpaceObjectManager;
class UUIManager;

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

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GameMode|UI Manager")
	TObjectPtr<UUIManager> _uiManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Space Size")
	float _spaceRadius = 1000.0f;

public:
	UFUNCTION()
	float GetSpaceRadius() { return _spaceRadius; }

protected:
	virtual void BeginPlay() override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
	UFUNCTION(BlueprintCallable, Category = "GameMode|Space Station")
	ASpaceStation* GetSpaceStation();

	UFUNCTION(BlueprintCallable, Category = "GameMode|Space Object Manager")
	USpaceObjectManager* GetSpaceObjectManager() { return _spaceObjectManager; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|UI Manager")
	UUIManager* GetUIManager() { return _uiManager; }
};
