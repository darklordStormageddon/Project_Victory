// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class E_INTERACT_TYPE : uint8
{
	// Panel
	None = 0 UMETA(DisplayName = "None"),
	Chair UMETA(DisplayName = "Chair"),
	DumpThrow UMETA(DisplayName = "DumpThrow"),
};

class CommonEnums
{
public:
	CommonEnums();
	~CommonEnums();

public:
	static FString GetFStringInteractEnum(E_INTERACT_TYPE InteractType);
};
