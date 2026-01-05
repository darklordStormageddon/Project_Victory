// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"

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

	InitializeSpaceObjectManager();
}


// Called every frame
void USpaceObjectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USpaceObjectManager::InitializeSpaceObjectManager()
{
	
}

void USpaceObjectManager::UpdateSpaceObject(FSpaceObjectData SpaceObjectData)
{
	if (!_spaceStation)
	{
		AJHSGameMode* OutGameMode = nullptr;
		if (!UStaticFunctionLibrary::GetGameMode(OutGameMode))
			return;

		_spaceStation = OutGameMode->GetSpaceStation();

		FSpaceObjectData _spaceStationData;
		_spaceStationData.SpaceObjectComponent = _spaceStation->GetSpaceObjectComponent();
		_spaceStationData.SpaceObjectType = E_SPACE_OBJECT_TYPE::SpaceStation;
		_spaceStationData.Location = _spaceStation->GetActorLocation();
		_spaceStationData.Rotator = _spaceStation->GetActorRotation();

		_spaceObjectMap.Add(_spaceStationData.SpaceObjectComponent, _spaceStationData);
	}

	// _spaceStation이 여전히 nullptr이면 오류 로그 후 리턴
	if (!_spaceStation)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceObjectManager: UpdateSpaceObject: _spaceStation is nullptr"));
		return;
	}

	if (!_spaceObjectMap.Contains(SpaceObjectData.SpaceObjectComponent))
	{
		_spaceObjectMap.Add(SpaceObjectData.SpaceObjectComponent, SpaceObjectData);
	}

	_spaceObjectMap[SpaceObjectData.SpaceObjectComponent] = SpaceObjectData;
}

void USpaceObjectManager::RemoveSpaceObject(TObjectPtr<USpaceObjectComponent> NewSpaceObjectPtr)
{
	if (_spaceObjectMap.Contains(NewSpaceObjectPtr))
	{
		_spaceObjectMap.Remove(NewSpaceObjectPtr);
	}
}

TArray<FSpaceObjectData> GetNearSpaceObjectArray(float MaxDixtance)
{
	TArray<FSpaceObjectData> _nearSpaceObjectDataArray;

	return _nearSpaceObjectDataArray;
}