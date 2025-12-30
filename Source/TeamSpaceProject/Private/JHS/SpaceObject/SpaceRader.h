// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceRader.generated.h"

class ASpaceObjectManager;

UCLASS()
class ASpaceRader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceRader();

private:
	TArray<TObjectPtr<AActor>> _raderObjectArray;

	FTimerHandle _updateTimerHandle;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Center")
	TObjectPtr<UStaticMeshComponent> _raderCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Space Center")
	TObjectPtr<AActor> _spaceCenter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Space Object Manager")
	TObjectPtr<ASpaceObjectManager> _spaceObjectManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Update")
	float _updateInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Size")
	float _spaceRadius = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Size")
	float _raderRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Rader Object")
	TObjectPtr<AActor> _raderObjectTemplate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateSpaceObject();
};
