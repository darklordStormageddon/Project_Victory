// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

#include "SpaceRader.generated.h"

UCLASS()
class ASpaceRader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpaceRader();

private:
	UPROPERTY()
	TObjectPtr<USpaceObjectManager> _spaceObjectManager;

	UPROPERTY()
	TMap<E_SPACE_OBJECT_TYPE, TSubclassOf<AActor>> _raderObjectMeshMap;

	UPROPERTY()
	FTimerHandle _updateTimerHandle;

	UPROPERTY()
	FString _fileFolderPath = "/Game/Main/PS_JHS/Resource/SpaceRaderMesh/";

	UPROPERTY()
	FString _fileHeaderName = "BP_RO";

	// 우주선 렌더링
	UPROPERTY()
	TArray<TObjectPtr<AActor>> _raderSpaceShipArray;

	UPROPERTY()
	int32 _spaceShipLastIndex = 0;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> _raderAsteroidArray;

	UPROPERTY()
	int32 _asteroidLastIndex = 0;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Debug")
	bool _isDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceRader|Components")
	TObjectPtr<UStaticMeshComponent> _rootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceRader|Rader")
	TObjectPtr<UStaticMeshComponent> _raderCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Rader")
	float _raderRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Space")
	TObjectPtr<AActor> _spaceCenter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Space")
	float _spaceRadius = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceRader|Update")
	float _updateInterval = 2.0f;

public:
	float GetMaxSpaceDistance() { return _spaceRadius; }

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

	TObjectPtr<AActor> GetRenderRaderObject(E_SPACE_OBJECT_TYPE SpaceObjectType);
};
