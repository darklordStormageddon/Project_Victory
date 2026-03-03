// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "JHS/GameControl/JHSPlayerState.h"

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

	UPROPERTY(ReplicatedUsing = "OnRep_PlayerStateArray")
	TArray<FPlayerStateData> _replicatedPlayerStateArray;

	UFUNCTION()
	void OnRep_PlayerStateArray();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializePlayerState(TObjectPtr<AJHSGameState> GameState);

	void UpdatePlayerState();

	void UpdatePlayerRadiation();

	bool TryRegistPlayer(TObjectPtr<AJHSPlayerState> PlayerState, FString PlayerName, E_REGIST_ERROR_TYPE& ErrorType);

	void IncreasePlayerRadiation(int32 CallerAssignedPlayerId, float DeltaTime);

private:
	bool TryGetPlayerStateData(int32 PlayerUID, FPlayerStateData*& OutPlayerStateData);

	void SyncPlayerStateToReplicated();
};
