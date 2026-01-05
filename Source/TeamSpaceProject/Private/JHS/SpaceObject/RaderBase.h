// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "RaderBase.generated.h"

USTRUCT(BlueprintType)
struct FRaderObjectData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_SPACE_OBJECT_TYPE SpaceObjectType;

	UPROPERTY()
	TSubclassOf<AActor> RaderObjectMesh;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> RaderObjectArray;

	UPROPERTY()
	int32 LastRaderObjectIndex = 0;
};

UCLASS()
class ARaderBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARaderBase();

private:
	UPROPERTY()
	FString FILE_FOLDER_PATH = "/Game/Main/PS_JHS/Resource/SpaceRaderMesh/";

	// 레이더 오브젝트 메쉬
	TMap<E_SPACE_OBJECT_TYPE, FRaderObjectData> _raderObjectDataMap;

	UPROPERTY()
	FTimerHandle _updateTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rader|Debug")
	bool _isDrawDebug = false;

	TObjectPtr<USpaceObjectManager> _spaceObjectManager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rader|Common")
	float _raderRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rader|Common")
	float _updateInterval = 2.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void InitializeRader() {}

	virtual FString GetFileHeaderName() { return ""; }

	void RenderSpaceObjectToRader(TObjectPtr<AActor> StandardActor, FVector RaderCenterLocation, float MaxDistance, float RaderRadiusRate);

private:
	void LoadRaderObjectMesh();

	void InitializeRaderBase();
};
