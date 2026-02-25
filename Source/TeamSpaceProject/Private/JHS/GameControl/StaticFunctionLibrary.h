// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StaticFunctionLibrary.generated.h"

class AJHSGameMode;
class AJHSPlayerController;
class AJHSGameState;
class USpaceObjectManager;
class UUIManager;
class UEventManager;

UCLASS()
class UStaticFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameMode")
    static bool TryGetGameMode(AJHSGameMode*& OutGameMode);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameMode")
    static bool TryGetPlayerController(AJHSPlayerController*& OutPlayerController);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameState")
    static bool TryGetGameState(AJHSGameState*& OutGameState);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|SpaceManager")
    static bool TryGetSpaceManager(USpaceManager*& OutSpaceManager);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|UIManager")
    static bool TryGetUIManager(UUIManager*& OutUIManager);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|EventManager")
    static bool TryGetEventManager(UEventManager*& OutEventManager);

    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameplayStatics")
    static float GetDeltaTime();

private:
    static bool TryGetWorld(UWorld*& OutWorld);
};
