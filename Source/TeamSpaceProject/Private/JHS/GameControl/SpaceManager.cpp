// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/Player/SpaceStation.h"
#include "JHS/SpaceObject/DriveSeatRader.h"
#include "JHS/SpaceObject/SpaceObjectComponent.h"

// Sets default values for this component's properties
USpaceManager::USpaceManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpaceManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USpaceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_isDrawDebug && GetSpaceStation() != nullptr)
	{
		DrawDebugSphere(GetWorld(), GetSpaceStation()->GetActorLocation(), _spaceRadius, 10, FColor::Yellow, false, DeltaTime * 1.01);
	}
	if (_isDrawDebug && _spaceShip != nullptr)
	{
		DrawDebugSphere(GetWorld(), _spaceShip->GetActorLocation(), _driveRaderRadius, 10, FColor::Blue, false, DeltaTime * 1.01);
	}
}

ASpaceStation* USpaceManager::GetSpaceStation()
{
	// 없으면 스캔해서 찾기
	if (_spaceStation == nullptr)
	{
		UWorld* _world = GetWorld();
		if (!_world)
		{
			UE_LOG(LogTemp, Error, TEXT("AJHSGameMode: _world is nullptr"));
			return nullptr;
		}

		TArray<AActor*> _foundSpaceStationArray;
		UGameplayStatics::GetAllActorsOfClass(_world, ASpaceStation::StaticClass(), _foundSpaceStationArray);
		if (_foundSpaceStationArray.Num() > 0)
		{
			_spaceStation = Cast<ASpaceStation>(_foundSpaceStationArray[0]);
		}
	}

	return _spaceStation;
}

void USpaceManager::InitializeDriveRader(TObjectPtr<ADriveSeatRader> DriveRader, TObjectPtr<AActor> SpaceShip)
{
	if (DriveRader != nullptr)
	{
		DriveRader->InitializeDriveRader(_driveRaderRadius);

		_spaceShip = SpaceShip;
	}
}

void USpaceManager::UpdateSpaceObject(FSpaceObjectData SpaceObjectData)
{
	if (!_spaceStation)
	{
		_spaceStation = GetSpaceStation();

		// 우주 정거장 초기화
		if (_spaceStation)
		{
			FSpaceObjectData _spaceStationData;
			_spaceStationData.SpaceObjectComponent = _spaceStation->GetSpaceObjectComponent();
			_spaceStationData.SpaceObjectType = E_SPACE_OBJECT_TYPE::SpaceStation;
			_spaceStationData.Location = _spaceStation->GetActorLocation();
			_spaceStationData.Rotator = _spaceStation->GetActorRotation();

			_spaceObjectMap.Add(_spaceStationData.SpaceObjectComponent, _spaceStationData);
		}
	}

	if (!_spaceObjectMap.Contains(SpaceObjectData.SpaceObjectComponent))
	{
		_spaceObjectMap.Add(SpaceObjectData.SpaceObjectComponent, SpaceObjectData);
	}

	_spaceObjectMap[SpaceObjectData.SpaceObjectComponent] = SpaceObjectData;
}

void USpaceManager::RemoveSpaceObject(TObjectPtr<USpaceObjectComponent> NewSpaceObjectPtr)
{
	if (NewSpaceObjectPtr == nullptr ||
		NewSpaceObjectPtr->GetOwner() == nullptr ||
		!NewSpaceObjectPtr->GetOwner()->HasAuthority())
		return;

	if (_spaceObjectMap.Contains(NewSpaceObjectPtr))
	{
		_spaceObjectMap.Remove(NewSpaceObjectPtr);
	}
}