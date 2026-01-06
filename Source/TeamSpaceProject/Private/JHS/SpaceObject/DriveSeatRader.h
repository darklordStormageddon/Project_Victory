// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/SpaceObject/RaderBase.h"
#include "DriveSeatRader.generated.h"

class USpaceObjectManager;

UCLASS()
class ADriveSeatRader : public ARaderBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADriveSeatRader();

private:
	UPROPERTY()
	TObjectPtr<USpaceObjectManager> _spaceObjectManager = nullptr;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rader|DriveSeatRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rader|DriveSeatRader|Components")
	TObjectPtr<UStaticMeshComponent> _spaceShipCenter = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rader|DriveSeatRader|Components")
	TObjectPtr<UStaticMeshComponent> _driveSeatRaderCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rader|Radius")
	TObjectPtr<AActor> _spaceShip = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rader|Radius")
	float _spaceShipDetectRadius = 1000.0f;

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
