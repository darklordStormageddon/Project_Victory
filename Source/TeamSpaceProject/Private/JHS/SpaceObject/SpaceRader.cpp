// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

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
		FVector _spaceObjectToRaderLocation = (_spaceObjectData.Location - _spaceCenter->GetActorLocation()) * _raderRate;

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
			_objectMesh = _objectMeshSpaceShip;
			_objectMeshArray = &_raderSpaceShipArray;
			_lastIndex = &_spaceShipLastIndex;
			break;
		case E_SPACE_OBJECT_TYPE::Asteroid:
			_objectMesh = _objectMeshAsteroid;
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