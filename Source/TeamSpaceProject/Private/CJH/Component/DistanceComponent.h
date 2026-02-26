// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DistanceComponent.generated.h"

class AJHSGameMode;
class UStaticFunctionLibrary;

class UEventManager;
class UEventOnStartStage;
class UEventOnEndStage;

class ASpaceStation;
class APSJ_Spaceship;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDistanceDamaged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UDistanceComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	TObjectPtr<ASpaceStation> SpaceStation = nullptr;

	APSJ_Spaceship* OwnerActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Distance")
	float CheckDistanceTime = 1.0f;

private:
	AJHSGameMode* InGameMode = nullptr;

	double SpaceRadius = 0.0;

	FTimerHandle MesureTimerHandle;

	UPROPERTY()
	TObjectPtr<AActor> TargetA = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> TargetB = nullptr;

	double LastDistance = 0.0;

	bool bDamageEnabled = false;

	UEventManager* EventManager = nullptr;

	FDelegateHandle OnStartStageHandle;
	FDelegateHandle OnEndStageHandle;

private:
	void MeasureDistance();

	double CalculateDistance(const FVector& PointA, const FVector& PointB);

	void HandleStartStage(UEventOnStartStage* Event);
	void HandleEndStage(UEventOnEndStage* Event);

public:	
	// Sets default values for this component's properties
	UDistanceComponent();

	UPROPERTY(BlueprintAssignable, Category = "Distance")
	FOnDistanceDamaged OnDistanceDamaged;

	void StartMeasure(AActor* InTargetA, AActor* InTargetB);
	void StopMeasure();

	double GetLastDistance() const { return LastDistance; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

		
};
