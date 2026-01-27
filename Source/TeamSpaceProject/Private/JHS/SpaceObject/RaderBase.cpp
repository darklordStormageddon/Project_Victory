// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/RaderBase.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// Sets default values
ARaderBase::ARaderBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARaderBase::BeginPlay()
{
	Super::BeginPlay();

	// 레이더 매쉬 캐싱
	LoadRaderObjectMesh();

	InitializeRaderBase();
}

// Called every frame
void ARaderBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARaderBase::RenderSpaceObjectToRader(TObjectPtr<AActor> StandardActor, TObjectPtr<UStaticMeshComponent> RaderCenter, float MaxDistance)
{
	if (!_spaceObjectManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceRader: SpaceObjectManager is nullptr"));
		return;
	}

	for (auto& Elem : _raderObjectDataMap)
	{
		Elem.Value.LastRaderObjectIndex = 0;
	}

	// 모든 우주 객체 반복
	TArray<FSpaceObjectData> _spaceObjectArray;
	auto _spaceObjectMap = _spaceObjectManager->GetSpaceObjectMap();
	_spaceObjectMap.GenerateValueArray(_spaceObjectArray);

	for (int32 i = 0; i < _spaceObjectArray.Num(); i++)
	{
		FSpaceObjectData _spaceObjectData = _spaceObjectArray[i];
		FVector _fromCenterLocation = _spaceObjectData.Location - StandardActor->GetActorLocation();
		// 레이더 범위 밖 제외
		if (_fromCenterLocation.Length() > MaxDistance)
			continue;

		if (!_raderObjectDataMap.Contains(_spaceObjectData.SpaceObjectType))
			continue;

		FRaderObjectData& _raderObjectData = _raderObjectDataMap[_spaceObjectData.SpaceObjectType];

		// 레이더 메쉬 선정
		TObjectPtr<AActor> _raderObject = nullptr;
		if (_raderObjectData.RaderObjectArray.Num() <= _raderObjectData.LastRaderObjectIndex)
		{
			float _spawnPosition = _raderObjectData.LastRaderObjectIndex * 100.0f;
			_raderObject = GetWorld()->SpawnActor<AActor>(_raderObjectData.RaderObjectMesh, FVector(_spawnPosition, _spawnPosition, _spawnPosition), FRotator::ZeroRotator);
			_raderObject->AttachToComponent(RaderCenter, FAttachmentTransformRules::KeepWorldTransform);
			_raderObject->SetActorScale3D(FVector(_raderMeshSize, _raderMeshSize, _raderMeshSize));
			_raderObjectData.RaderObjectArray.Add(_raderObject);
		}

		_raderObject = _raderObjectData.RaderObjectArray[_raderObjectData.LastRaderObjectIndex];
		_raderObjectData.LastRaderObjectIndex++;

		if (_raderObject == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ARaderBase: RaderObject is nullptr"));
			return;
		}

		_raderObject->SetActorHiddenInGame(false);
		_raderObject->SetActorEnableCollision(true);
		_raderObject->SetActorTickEnabled(true);

		// 레이더 좌표 설정
		float _raderRate = _raderRadius / MaxDistance;
		FVector _raderLocation = _fromCenterLocation * _raderRate;

		_raderObject->SetActorRelativeLocation(_raderLocation);
		_raderObject->SetActorRelativeRotation(_spaceObjectData.Rotator);

		if (_isDrawDebug && HasAuthority())
		{
			DrawDebugLine(GetWorld(), RaderCenter->GetComponentLocation(), _raderObject->GetActorLocation(), FColor::Red, false, _updateInterval);
		}
	}

	// Deactivate left mesh
	TObjectPtr<AActor> _leftMesh = nullptr;
	for (auto& Elem : _raderObjectDataMap)
	{
		int _lastIndex = Elem.Value.LastRaderObjectIndex;
		for (int i = _lastIndex; i < Elem.Value.RaderObjectArray.Num(); i++)
		{
			_leftMesh = Elem.Value.RaderObjectArray[i];
			if (_leftMesh == nullptr)
				continue;

			_leftMesh->SetActorHiddenInGame(true);
			_leftMesh->SetActorEnableCollision(false);
			_leftMesh->SetActorTickEnabled(false);
		}
	}

	// 딜레이 후 반복
	GetWorld()->GetTimerManager().SetTimer(
		_updateTimerHandle,
		FTimerDelegate::CreateUObject(this, &ARaderBase::RenderSpaceObjectToRader, StandardActor, RaderCenter, MaxDistance),
		_updateInterval,
		false
	);
}

void ARaderBase::LoadRaderObjectMesh()
{
	_raderObjectDataMap.Empty();

	for (int32 i = 0; i < (int32)E_SPACE_OBJECT_TYPE::SpaceShip + 1; i++)
	{
		E_SPACE_OBJECT_TYPE _spaceObjectType = (E_SPACE_OBJECT_TYPE)i;
		FString _typeName = "";

		switch (_spaceObjectType)
		{
		case E_SPACE_OBJECT_TYPE::SpaceStation:
			_typeName = "SpaceStation";
			break;
		case E_SPACE_OBJECT_TYPE::SpaceGarbage:
			_typeName = "SpaceGarbage";
			break;
		case E_SPACE_OBJECT_TYPE::Asteroid:
			_typeName = "Asteroid";
			break;
		case E_SPACE_OBJECT_TYPE::Enemy:
			_typeName = "Enemy";
			break;
		case E_SPACE_OBJECT_TYPE::SpaceShip:
			_typeName = "SpaceShip";
			break;
		default:
			continue;
		}
		
		FString _blueprintName = this->GetFileHeaderName() + _typeName;
		FString _blueprintPath = ConstantLibrary::Resource.SpaceObject.RADER_MESH_FOLDER_PATH + this->GetFilePathName() + _blueprintName + "." + _blueprintName + "_C";

		UClass* _blueprintClass = StaticLoadClass(AActor::StaticClass(), nullptr, *_blueprintPath);

		if (!_blueprintClass)
		{
			UE_LOG(LogTemp, Error, TEXT("RaderBase: Blueprint class is nullptr in [%s]"), *_blueprintPath);
			continue;
		}

		FRaderObjectData _raderObjectData;
		_raderObjectData.SpaceObjectType = _spaceObjectType;
		_raderObjectData.RaderObjectMesh = _blueprintClass;
		_raderObjectData.RaderObjectArray.Empty();
		_raderObjectData.LastRaderObjectIndex = 0;

		_raderObjectDataMap.Add(_raderObjectData.SpaceObjectType, _raderObjectData);
		//UE_LOG(LogTemp, Warning, TEXT("SpaceRader: Loaded blueprint [%s] for type [%s]"), *_blueprintPath, *_typeName);
	}
}

void ARaderBase::InitializeRaderBase()
{
	USpaceObjectManager* _outSpaceObjectManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetSpaceObjectManager(_outSpaceObjectManager))
		return;

	_spaceObjectManager = _outSpaceObjectManager;

	InitializeRader();
}