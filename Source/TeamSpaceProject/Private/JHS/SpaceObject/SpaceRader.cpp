// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// Sets default values
ASpaceRader::ASpaceRader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	_rootComponent->SetupAttachment(RootComponent);

	_raderCenter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RaderCenter"));
	_raderCenter->SetupAttachment(_rootComponent);
}

// Called when the game starts or when spawned
void ASpaceRader::BeginPlay()
{
	Super::BeginPlay();

	InitializeSpaceRader();
	UpdateSpaceObject();
}

// Called every frame
void ASpaceRader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// _spaceRadius 만큼 DebugDrawSphere 그리기
	DrawDebugSphere(GetWorld(), _spaceCenter->GetActorLocation(), _spaceRadius, 10, FColor::Yellow, false, DeltaTime * 1.01);

	// _raderRadius 만큼 DebugDrawSphere 그리기
	DrawDebugSphere(GetWorld(), _raderCenter->GetComponentLocation(), _raderRadius, 10, FColor::Blue, false, DeltaTime * 1.01);
}

void ASpaceRader::InitializeSpaceRader()
{
	USpaceObjectManager* OutSpaceObjectManager = nullptr;
	if (!UStaticFunctionLibrary::GetSpaceObjectManager(OutSpaceObjectManager))
		return;

	_spaceObjectManager = OutSpaceObjectManager;

	// 레이더 메쉬 캐싱
	LoadRaderObjectMesh();
}

void ASpaceRader::LoadRaderObjectMesh()
{
	// E_SPACE_OBJECT_TYPE의 모든 값 반복
	for (int32 i = 0; i < (int32)E_SPACE_OBJECT_TYPE::SpaceShip + 1; i++)
	{
		E_SPACE_OBJECT_TYPE _spaceObjectType = (E_SPACE_OBJECT_TYPE)i;
		FString _typeName = "";
		
		// 타입 이름 가져오기
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

		// BP_RO[타입명]_C 형식의 블루프린트 경로 생성
		FString _blueprintName = _fileHeaderName + _typeName;
		FString _blueprintPath = _fileFolderPath + _blueprintName + "." + _blueprintName + "_C";
		
		// 블루프린트 클래스 로드
		UClass* _blueprintClass = StaticLoadClass(AActor::StaticClass(), nullptr, *_blueprintPath);
		
		if (!_blueprintClass)
		{
			UE_LOG(LogTemp, Error, TEXT("SpaceRader: Blueprint class is nullptr in [%s]"), *_blueprintPath);
			continue;
		}

		// 맵에 캐싱
		_raderObjectMeshMap.Add(_spaceObjectType, _blueprintClass);
		UE_LOG(LogTemp, Warning, TEXT("SpaceRader: Loaded blueprint [%s] for type [%s]"), *_blueprintPath, *_typeName);
	}
}

void ASpaceRader::UpdateSpaceObject()
{
	if (!_spaceObjectManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceRader: SpaceObjectManager is nullptr"));
		return;
	}

	_spaceShipLastIndex = 0;
	_asteroidLastIndex = 0;

	float _raderRate = _raderRadius / _spaceRadius;

	auto _spaceObjectMap = _spaceObjectManager->GetSpaceObjectMap();

	TArray<FSpaceObjectData> _spaceObjectArray;
	_spaceObjectMap.GenerateValueArray(_spaceObjectArray);
	for (int32 i = 0; i < _spaceObjectArray.Num(); i++)
	{
		FSpaceObjectData _spaceObjectData = _spaceObjectArray[i];
		FVector _centerToObject = _spaceObjectData.Location - _spaceCenter->GetActorLocation();

		if (_centerToObject.Length() >= _spaceRadius)
			continue;

		FVector _spaceObjectToRaderLocation = _centerToObject * _raderRate;

		TObjectPtr<AActor> _renderRaderObject = GetRenderRaderObject(_spaceObjectData.SpaceObjectType);
		if (_renderRaderObject)
		{
			_renderRaderObject->SetActorLocation(_raderCenter->GetComponentLocation() + _spaceObjectToRaderLocation);
			_renderRaderObject->SetActorRotation(_spaceObjectData.Rotator);
			
			if (_isDrawDebug && HasAuthority())
			{
				DrawDebugLine(GetWorld(), _raderCenter->GetComponentLocation(), _renderRaderObject->GetActorLocation(), FColor::Red, false, _updateInterval);
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		_updateTimerHandle,
		FTimerDelegate::CreateUObject(this, &ASpaceRader::UpdateSpaceObject),
		_updateInterval,
		false
	);
}

TObjectPtr<AActor> ASpaceRader::GetRenderRaderObject(E_SPACE_OBJECT_TYPE SpaceObjectType)
{
	// 타입에 따라 원본 데이터를 직접 수정
	TObjectPtr<AActor> _objectMesh = nullptr;
	TArray<TObjectPtr<AActor>>* _objectMeshArray = nullptr;
	int32* _lastIndex = nullptr;

	switch (SpaceObjectType)
	{
		case E_SPACE_OBJECT_TYPE::SpaceShip:
			_objectMesh = _raderObjectMeshMap[SpaceObjectType].GetDefaultObject();
			_objectMeshArray = &_raderSpaceShipArray;
			_lastIndex = &_spaceShipLastIndex;
			break;
		case E_SPACE_OBJECT_TYPE::Asteroid:
			_objectMesh = _raderObjectMeshMap[SpaceObjectType].GetDefaultObject();
			_objectMeshArray = &_raderAsteroidArray;
			_lastIndex = &_asteroidLastIndex;
			break;
		default:
			break;
	}

	if (!_objectMesh || !_objectMeshArray || !_lastIndex)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceRader: ObjectMesh, MeshArray, or LastIndex is nullptr"));
		return nullptr;
	}
	
	TObjectPtr<AActor> _raderObject = nullptr;
	if (_objectMeshArray->Num() <= *_lastIndex)
	{
		float SpawnPosition = (*_lastIndex) * 100.0f;
		_raderObject = GetWorld()->SpawnActor<AActor>(_objectMesh->GetClass(), FVector(SpawnPosition, SpawnPosition, SpawnPosition), FRotator::ZeroRotator);
		_raderObject->SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
		_objectMeshArray->Add(_raderObject);
	}

	_raderObject = (*_objectMeshArray)[*_lastIndex];
	//UE_LOG(LogTemp, Warning, TEXT("%s : LastIndex: %d"), (int32)SpaceObjectType, *_lastIndex);
	(*_lastIndex)++;
	return _raderObject;
}