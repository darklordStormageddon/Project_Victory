// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StaticFunctionLibrary.generated.h"

class AJHSGameMode;
class AJHSGameState;

UCLASS()
class UStaticFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
    static bool TryGetWorld(UWorld*& OutWorld);
	
public:
    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameMode")
    static bool TryGetGameMode(AJHSGameMode*& OutGameMode);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameState")
    static bool TryGetGameState(AJHSGameState*& OutGameState);
    
    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|SpaceObjectManager")
    static bool TryGetSpaceObjectManager(USpaceObjectManager*& OutSpaceObjectManager);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|UIManager")
    static bool TryGetUIManager(UUIManager*& OutUIManager);
};
