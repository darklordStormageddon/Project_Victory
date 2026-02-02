// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "JHSGameMode.generated.h"

class UUIManager;
class UEventManager;
class USpaceManager;

UCLASS()
class AJHSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AJHSGameMode();

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "GameMode|Manager")
	TObjectPtr<UUIManager> _uiManager = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = "GameMode|Manager")
	TObjectPtr<UEventManager> _eventManager = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = "GameMode|Manager")
	TObjectPtr<USpaceManager> _spaceManager = nullptr;

public:
	UFUNCTION(BlueprintCallable, Category = "GameMode|UI Manager")
	UUIManager* GetUIManager() { return _uiManager; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Event Manager")
	UEventManager* GetEventManager() { return _eventManager; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Space Manager")
	USpaceManager* GetSpaceManager() { return _spaceManager; }

protected:
	virtual void BeginPlay() override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
	UFUNCTION(BlueprintCallable, Category = "GameMode|Game")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "GameMode|Game")
	void StartStage(int32 Stage);
};
