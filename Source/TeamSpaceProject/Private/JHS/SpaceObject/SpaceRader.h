// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "SpaceRader.generated.h"

class AJHSGameMode;
class ASpaceStation;

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
class ASpaceRader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceRader();

private:
	UPROPERTY()
	TObjectPtr<USpaceObjectManager> _spaceObjectManager = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> _spaceStation = nullptr;

	UPROPERTY()
	float _spaceRadius = 0.0f;

	UPROPERTY()
	float _raderRate = 0.0f;

	UPROPERTY()
	FTimerHandle _updateTimerHandle;

	UPROPERTY()
	FString FILE_FOLDER_PATH = "/Game/Main/PS_JHS/Resource/SpaceRaderMesh/";

	UPROPERTY()
	FString FILE_HEADER_NAME = "BP_RO";

	// 레이더 오브젝트
	TMap<E_SPACE_OBJECT_TYPE, FRaderObjectData> _raderObjectDataMap;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Debug")
	bool _isDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceRader|Rader")
	TObjectPtr<UStaticMeshComponent> _raderCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Rader")
	float _raderRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Update")
	float _updateInterval = 2.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void InitializeSpaceRader();

	void LoadRaderObjectMesh();

	void UpdateSpaceObject();

	void RenderSpaceObjectToRader(FSpaceObjectData SpaceObjectData);
};
