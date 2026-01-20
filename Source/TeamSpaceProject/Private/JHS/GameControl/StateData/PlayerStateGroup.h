// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"

#include "PlayerStateGroup.generated.h"

class AJHSGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPlayerStateGroup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerStateGroup();

private:
	UPROPERTY()
	TObjectPtr<AJHSGameState> _gameState = nullptr;

	UPROPERTY()
	TMap<int32, FPlayerStateData> _playerStateMap;

public:
	int32 GetPlayerCount() { return _playerStateMap.Num(); }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializePlayerState(TObjectPtr<AJHSGameState> GameState, TArray<FPlayerStateData> PlayerStateArray, float MaxPlayerRadiation);

	void UpdatePlayerState();

	void IncreasePlayerRadiation(int32 PlayerIdx, float IncreaseValue);
};
