// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/RaderBase.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/CommonEnums.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// Sets default values
ARaderBase::ARaderBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	_rootComponent->SetupAttachment(RootComponent);

	_raderCenter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RaderCenter"));
	_raderCenter->SetupAttachment(_rootComponent);
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

void ARaderBase::StartRenderRader(FRaderData RaderData)
{
	_raderData = RaderData;
	if (_raderData.StandardActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RaderBase: Standard actor is nullptr"));
		return;
	}

	RenderSpaceObjectToRader();
}

void ARaderBase::RenderSpaceObjectToRader()
{
	if (!_spaceManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceRader: SpaceManager is nullptr"));
		return;
	}

	for (auto& Elem : _raderObjectDataMap)
	{
		Elem.Value.LastRaderObjectIndex = 0;
	}

	// 모든 우주 객체 반복
	TArray<FSpaceObjectData> _spaceObjectArray;
	auto _spaceObjectMap = _spaceManager->GetSpaceObjectMap();
	_spaceObjectMap.GenerateValueArray(_spaceObjectArray);

	for (int32 i = 0; i < _spaceObjectArray.Num(); i++)
	{
		FSpaceObjectData _spaceObjectData = _spaceObjectArray[i];
		FVector _fromCenterLocation = _spaceObjectData.Location - _raderData.StandardActor->GetActorLocation();
		// 레이더 범위 밖 제외
		if (_fromCenterLocation.Length() > _raderData.RaderRenderRadius)
			continue;

		if (!_raderObjectDataMap.Contains(_spaceObjectData.SpaceObjectType))
			continue;

		FRaderObjectData& _raderObjectData = _raderObjectDataMap[_spaceObjectData.SpaceObjectType];

		// 레이더 메쉬 선정
		TObjectPtr<AActor> _raderObject = nullptr;
		if (_raderObjectData.RaderObjectArray.Num() <= _raderObjectData.LastRaderObjectIndex)
		{
			float _spawnPosition = _raderObjectData.LastRaderObjectIndex * 100.0f;
			UWorld* _world = GetWorld();
			TSubclassOf<AActor> _raderObjectMesh = _raderObjectData.RaderObjectMesh;
			if (_world != nullptr && _raderObjectMesh != nullptr)
			{
				_raderObject = _world->SpawnActor<AActor>(
					_raderObjectMesh,
					FVector(_spawnPosition, _spawnPosition, _spawnPosition),
					FRotator::ZeroRotator
				);

				// SpawnActor 실패 체크를 먼저
				if (_raderObject == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("ARaderBase: SpawnActor failed"));
					continue; // return 대신 continue로 다음 객체 처리
				}

				_raderObject->AttachToComponent(_raderCenter, FAttachmentTransformRules::KeepWorldTransform);
				_raderObject->SetActorScale3D(FVector(_raderMeshSize, _raderMeshSize, _raderMeshSize));
				_raderObjectData.RaderObjectArray.Add(_raderObject);
			}
			else
			{
				continue; // world나 mesh가 null이면 스킵
			}
		}

		_raderObject = _raderObjectData.RaderObjectArray[_raderObjectData.LastRaderObjectIndex];

		// 배열에서 꺼낸 후에도 유효성 검사 (GC로 인한 dangling 방어)
		if (!IsValid(_raderObject))
		{
			UE_LOG(LogTemp, Error, TEXT("ARaderBase: RaderObject is invalid"));
			_raderObjectData.RaderObjectArray.RemoveAt(_raderObjectData.LastRaderObjectIndex);
			continue;
		}

		_raderObject->SetActorHiddenInGame(false);
		_raderObject->SetActorEnableCollision(true);
		_raderObject->SetActorTickEnabled(true);

		// 레이더 좌표 설정
		float _raderRate = _raderRadius / _raderData.RaderRenderRadius;
		FVector _raderLocation = _fromCenterLocation * _raderRate;

		_raderObject->SetActorRelativeLocation(_raderLocation);
		_raderObject->SetActorRelativeRotation(_spaceObjectData.Rotator);
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
	GetWorld()->GetTimerManager().SetTimer(_updateTimerHandle, this, &ARaderBase::RenderSpaceObjectToRader, _updateInterval, false
	);
}

void ARaderBase::LoadRaderObjectMesh()
{
	_raderObjectDataMap.Empty();

	for (int32 i = 0; i < (int32)E_SPACE_OBJECT_TYPE::SpaceShip + 1; i++)
	{
		E_SPACE_OBJECT_TYPE _spaceObjectType = (E_SPACE_OBJECT_TYPE)i;
		FString _blueprintName = this->GetFileHeaderName() + CommonEnums::GetEnum2FString<E_SPACE_OBJECT_TYPE>(_spaceObjectType);
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
	USpaceManager* _outSpaceManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetSpaceManager(_outSpaceManager))
		return;

	_spaceManager = _outSpaceManager;

	InitializeRader();
}