// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/SpaceObject/RaderBase.h"

#include "SpaceRader.generated.h"

class ASpaceStation;

UCLASS()
class ASpaceRader : public ARaderBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceRader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	void InitializeRader() override;

	FString GetFilePathName() override;

	FString GetFileHeaderName() override;
};
