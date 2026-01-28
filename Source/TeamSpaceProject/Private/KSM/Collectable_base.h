// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "Collectable_base.generated.h"

class AJHSGameState;

UCLASS()
class ACollectable_base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACollectable_base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh;

	AJHSGameState* _outGameState = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UMaterialInterface* OverlayMaterial;

	//디스플레이용 + 데이터테이블 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ore")
	E_ELEMENT_TYPE ElementType;

public:

	UFUNCTION(BlueprintCallable, Category = "Collect")
	void Add_Elem_To_GameState(E_ELEMENT_TYPE type, int32 amount);
};
