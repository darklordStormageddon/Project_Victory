// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StaticFunctionLibrary.generated.h"

class AJHSGameMode;

UCLASS()
class UStaticFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|GameMode")
    static bool GetGameMode(AJHSGameMode*& OutGameMode);
    
    UFUNCTION(BlueprintCallable, Category = "StaticFunctionLibrary|SpaceObjectManager")
    static bool GetSpaceObjectManager(USpaceObjectManager*& OutSpaceObjectManager);
};
