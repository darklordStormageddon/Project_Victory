// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "JHSPlayerState.generated.h"

UENUM(BlueprintType)
enum class E_REGIST_ERROR_TYPE : uint8
{
	NullPlayerState = 0 UMETA(DisplayName = "NullPlayerState"),
	DuplicatedUID UMETA(DisplayName = "DuplicatedUID"),
	DuplicatedName UMETA(DisplayName = "DuplicatedName"),

	NONE UMETA(DisplayName = "NONE"),
};

UCLASS()
class AJHSPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	UPROPERTY(Replicated)
	int32 _assignedPlayerId = -1;

public:
	/** 서버에서 접속 시 발급한 ID (복제). UI/Interact 호출자 구별용. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	int32 GetAssignedPlayerId() const { return _assignedPlayerId; }

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	bool TryRegistPlayer(FString Name, E_REGIST_ERROR_TYPE& OutErrorType);

	void SetAssignedPlayerId(int32 AssignedPlayerId);

private:
	void OpenCommonInfoUI();
};
