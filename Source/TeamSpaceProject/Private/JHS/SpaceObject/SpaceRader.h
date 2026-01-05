// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/SpaceObject/RaderBase.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "SpaceRader.generated.h"

class AJHSGameMode;
class ASpaceStation;

UCLASS()
class ASpaceRader : public ARaderBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceRader();

private:
	UPROPERTY()
	TObjectPtr<AActor> _spaceStation = nullptr;

	UPROPERTY()
	float _spaceRadius = 0.0f;

	UPROPERTY()
	float _raderRate = 0.0f;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rader|SpaceRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rader|SpaceRader|Rader")
	TObjectPtr<UStaticMeshComponent> _raderCenter = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	void InitializeRader() override;

	FString GetFileHeaderName() override;
};
