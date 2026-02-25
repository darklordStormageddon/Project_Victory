// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "JHSGameMode.generated.h"

class UEventManager;
class USpaceManager;
class UShopManager;
class AJHSGameState;

USTRUCT(BlueprintType)
struct FEquipTurretData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	E_TURRET_POSITION TurretPosition = E_TURRET_POSITION::END;

	UPROPERTY(EditAnywhere)
	E_AMMO_TYPE AmmoType = E_AMMO_TYPE::NONE;
};

UCLASS()
class AJHSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AJHSGameMode();

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "GameMode|Manager")
	TObjectPtr<UEventManager> _eventManager = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = "GameMode|Manager")
	TObjectPtr<USpaceManager> _spaceManager = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = "GameMode|Manager")
	TObjectPtr<UShopManager> _shopManager = nullptr;

	UPROPERTY()
	TObjectPtr<AJHSGameState> _cachedGameState = nullptr;

	UPROPERTY()
	bool _isGameStarted = false;

	UPROPERTY()
	bool _isStageStarted = false;

	UPROPERTY()
	int32 _currentStage = 0;

	// Start Equip Turret
	UPROPERTY(EditAnywhere, Category = "GameMode|Start Equip Turret")
	TArray<FEquipTurretData> _startEquipTurretArray;

public:
	UFUNCTION(BlueprintCallable, Category = "GameMode|Event Manager")
	UEventManager* GetEventManager() { return _eventManager; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Space Manager")
	USpaceManager* GetSpaceManager() { return _spaceManager; }

protected:
	virtual void BeginPlay() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	UFUNCTION(BlueprintCallable, Category = "GameMode|Game")
	void StartGame(AActor* Caller);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Game")
	void EndGame(AActor* Caller);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Stage")
	void StartNextStage(AActor* Caller);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Stage")
	void EndStage(AActor* Caller);

private:
	UFUNCTION()
	bool CheckIsServerCaller(AActor* Caller);

	UFUNCTION()
	bool TryGetGameState(AJHSGameState*& OutGameState);
};
