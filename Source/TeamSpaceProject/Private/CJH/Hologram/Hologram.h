// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/GameplayStatics.h"

#include "Hologram.generated.h"

UCLASS()
class AHologram : public AActor
{
	GENERATED_BODY()
private:
	FTimerHandle HologramTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	float HologramDuration = 2.0f;

	TArray <AActor*> SavedAllObject;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	TArray <TSubclassOf<AActor>> SavedShipObject;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	TArray <TSubclassOf<AActor>> SavedAsternoidObject;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	TArray <TSubclassOf<AActor>> SavedWasteObject;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	TArray <TSubclassOf<AActor>> SavedEnemyObject;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	TArray <TSubclassOf<AActor>> SavedStationObject;

	UPROPERTY(EditAnywhere, Category = "Hologram")
	TMap<AActor*, FVector> TemporaryInfo;

private:
	void GetAllActors();
	void CallActorInfo();
	void HologramTimer();
	void SetHologram();

public:	
	// Sets default values for this actor's properties
	AHologram();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
