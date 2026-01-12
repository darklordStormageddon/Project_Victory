// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AS_BodyComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAS_BodyComponent : public USceneComponent
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere, Category = "Body")
	UStaticMeshComponent* BodyMesh;

public:	
	// Sets default values for this component's properties
	UAS_BodyComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
