// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "JHS/SpaceObject/SpaceObjectBase.h"

// Sets default values for this component's properties
USpaceObjectManager::USpaceObjectManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpaceObjectManager::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void USpaceObjectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USpaceObjectManager::UpdateSpaceObject(FSpaceObjectData SpaceObjectData)
{
	if (!_spaceObjectMap.Contains(SpaceObjectData.SpaceObjectPtr))
	{
		_spaceObjectMap.Add(SpaceObjectData.SpaceObjectPtr, SpaceObjectData);
	}

	_spaceObjectMap[SpaceObjectData.SpaceObjectPtr] = SpaceObjectData;
}

void USpaceObjectManager::RemoveSpaceObject(TObjectPtr<ASpaceObjectBase> NewSpaceObjectPtr)
{
	if (_spaceObjectMap.Contains(NewSpaceObjectPtr))
	{
		_spaceObjectMap.Remove(NewSpaceObjectPtr);
	}
}