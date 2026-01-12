// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SpaceStation.generated.h"

class USpaceObjectComponent;

UCLASS()
class ASpaceStation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceStation();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Rader")
	TObjectPtr<USpaceObjectComponent> _spaceObjectComponent = nullptr;

public:
	TObjectPtr<USpaceObjectComponent> GetSpaceObjectComponent() { return _spaceObjectComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
