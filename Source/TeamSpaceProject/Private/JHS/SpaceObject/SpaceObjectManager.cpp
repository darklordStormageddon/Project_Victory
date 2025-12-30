// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "JHS/SpaceObject/SpaceObjectBase.h"

// Sets default values
ASpaceObjectManager::ASpaceObjectManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASpaceObjectManager::BeginPlay()
{
	Super::BeginPlay();
	
	for (TObjectPtr<ASpaceObjectBase> _spaceObject : _initializeSpaceObjectArray)
	{
		_spaceObject->InitializeSpaceObject(this);
	}
}

// Called every frame
void ASpaceObjectManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpaceObjectManager::UpdateSpaceObject(FSpaceObjectData SpaceObjectData)
{
	if (!_spaceObjectMap.Contains(SpaceObjectData.SpaceObjectPtr))
	{
		_spaceObjectMap.Add(SpaceObjectData.SpaceObjectPtr, SpaceObjectData);
	}

	_spaceObjectMap[SpaceObjectData.SpaceObjectPtr] = SpaceObjectData;
}

void ASpaceObjectManager::RemoveSpaceObject(TObjectPtr<ASpaceObjectBase> NewSpaceObjectPtr)
{
	if (_spaceObjectMap.Contains(NewSpaceObjectPtr))
	{
		_spaceObjectMap.Remove(NewSpaceObjectPtr);
	}
}