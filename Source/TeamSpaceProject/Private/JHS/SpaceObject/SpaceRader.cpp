// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
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
}

// Called every frame
void ASpaceRader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// _spaceRadius 만큼 DebugDrawSphere 그리기
	if (_spaceStation)
	{
		DrawDebugSphere(GetWorld(), _spaceStation->GetActorLocation(), _spaceRadius, 10, FColor::Yellow, false, DeltaTime * 1.01);
	}

	// _raderRadius 만큼 DebugDrawSphere 그리기
	DrawDebugSphere(GetWorld(), _raderCenter->GetComponentLocation(), _raderRadius, 10, FColor::Blue, false, DeltaTime * 1.01);
}

void ASpaceRader::InitializeSpaceRader()
{
	AJHSGameMode* OutGameMode = nullptr;
	if (!UStaticFunctionLibrary::GetGameMode(OutGameMode))
		return;

	_spaceObjectManager = OutGameMode->GetSpaceObjectManager();
	_spaceStation = Cast<AActor>(OutGameMode->GetSpaceStation());
	_spaceRadius = OutGameMode->GetSpaceRadius();
	_raderRate = _raderRadius / _spaceRadius;

	// 레이더 메쉬 캐싱
	LoadRaderObjectMesh();

	// 레이더 표시 시작
	UpdateSpaceObject();
}

void ASpaceRader::LoadRaderObjectMesh()
{
	_raderObjectDataMap.Empty();

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
		FString _blueprintName = this->FILE_HEADER_NAME + _typeName;
		FString _blueprintPath = this->FILE_FOLDER_PATH + _blueprintName + "." + _blueprintName + "_C";
		
		// 블루프린트 클래스 로드
		UClass* _blueprintClass = StaticLoadClass(AActor::StaticClass(), nullptr, *_blueprintPath);
		
		if (!_blueprintClass)
		{
			UE_LOG(LogTemp, Error, TEXT("SpaceRader: Blueprint class is nullptr in [%s]"), *_blueprintPath);
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

void ASpaceRader::UpdateSpaceObject()
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

	// 거점 우주 정거장 표시
	FSpaceObjectData _spaceStationData;
	_spaceStationData.SpaceObjectType = E_SPACE_OBJECT_TYPE::SpaceStation;
	_spaceStationData.Location = _spaceStation->GetActorLocation();
	_spaceStationData.Rotator = _spaceStation->GetActorRotation();
	RenderSpaceObjectToRader(_spaceStationData);

	// 우주 부유물 표시
	auto _spaceObjectMap = _spaceObjectManager->GetSpaceObjectMap();
	TArray<FSpaceObjectData> _spaceObjectArray;
	_spaceObjectMap.GenerateValueArray(_spaceObjectArray);
	for (int32 i = 0; i < _spaceObjectArray.Num(); i++)
	{
		FSpaceObjectData _spaceObjectData = _spaceObjectArray[i];
		RenderSpaceObjectToRader(_spaceObjectData);
	}

	GetWorld()->GetTimerManager().SetTimer(
		_updateTimerHandle,
		FTimerDelegate::CreateUObject(this, &ASpaceRader::UpdateSpaceObject),
		_updateInterval,
		false
	);
}

void ASpaceRader::RenderSpaceObjectToRader(FSpaceObjectData SpaceObjectData)
{
	// 우주 중심에서 우주 부유물의 거리 벡터
	FVector _centerToObject = SpaceObjectData.Location - _spaceStation->GetActorLocation();
	// 맵 범위 이탈
	if (_centerToObject.Length() >= _spaceRadius)
		return;

	// 우주 부유물 타입의 렌더 메쉬 선정
	FRaderObjectData& _raderObjectData = _raderObjectDataMap[SpaceObjectData.SpaceObjectType];

	TObjectPtr<AActor> _raderObject = nullptr;
	if (_raderObjectData.RaderObjectArray.Num() <= _raderObjectData.LastRaderObjectIndex)
	{
		float SpawnPosition = _raderObjectData.LastRaderObjectIndex * 100.0f;
		_raderObject = GetWorld()->SpawnActor<AActor>(_raderObjectData.RaderObjectMesh, FVector(SpawnPosition, SpawnPosition, SpawnPosition), FRotator::ZeroRotator);
		_raderObject->SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
		_raderObjectData.RaderObjectArray.Add(_raderObject);
	}

	_raderObject = _raderObjectData.RaderObjectArray[_raderObjectData.LastRaderObjectIndex];
	_raderObjectData.LastRaderObjectIndex++;
	
	if (_raderObject == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ASpaceRader: RaderObject is nullptr"));
		return;
	}

	// 렌더 메쉬 좌표, 방향 표시
	FVector _spaceObjectToRaderLocation = _centerToObject * _raderRate;


	_raderObject->SetActorLocation(_raderCenter->GetComponentLocation() + _spaceObjectToRaderLocation);
	_raderObject->SetActorRotation(SpaceObjectData.Rotator);

	if (_isDrawDebug && HasAuthority())
	{
		DrawDebugLine(GetWorld(), _raderCenter->GetComponentLocation(), _raderObject->GetActorLocation(), FColor::Red, false, _updateInterval);
	}
}