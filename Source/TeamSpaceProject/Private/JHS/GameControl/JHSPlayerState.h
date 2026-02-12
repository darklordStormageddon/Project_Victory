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

public:
	bool TryRegistPlayer(FString Name, E_REGIST_ERROR_TYPE& OutErrorType);
};
