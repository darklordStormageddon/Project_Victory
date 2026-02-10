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

    virtual void PreLogin(const FString& Options, const FString& Address,
        const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual FString InitNewPlayer(APlayerController* NewPlayerController,
        const FUniqueNetIdRepl& UniqueId, const FString& Options,
        const FString& Portal = TEXT("")) override;
};