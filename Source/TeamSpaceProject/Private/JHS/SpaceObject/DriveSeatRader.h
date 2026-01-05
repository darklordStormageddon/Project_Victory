// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DriveSeatRader.generated.h"

class USpaceObjectManager;

UCLASS()
class ADriveSeatRader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADriveSeatRader();

private:
	UPROPERTY()
	TObjectPtr<USpaceObjectManager> _spaceObjectManager = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceRader|Rader")
	TObjectPtr<UStaticMeshComponent> _spaceShipCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DriveSeatRader|Rader")
	float _raderRadius = 100.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void InitializeDriveSeatRader();

	void UpdateDriveSeatRader();
};
