// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"

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

void ASpaceRader::UpdateSpaceObject()
{
	if (!_spaceObjectManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceRader: SpaceObjectManager is nullptr"));
		return;
	}

	float _raderRate = _raderRadius / _spaceRadius;

	auto _spaceObjectMap = _spaceObjectManager->GetSpaceObjectMap();
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpaceRader: SpaceObjectMap size: %d"), _spaceObjectMap.Num());
	}

	TArray<FSpaceObjectData> _spaceObjectArray;
	_spaceObjectMap.GenerateValueArray(_spaceObjectArray);
	for (int32 i = 0; i < _spaceObjectArray.Num(); i++)
	{
		FSpaceObjectData _spaceObjectData = _spaceObjectArray[i];
		FVector _spaceObjectToRaderLocation = (_spaceObjectData.Location - _spaceCenter->GetActorLocation()) * _raderRate;

		if (_raderObjectArray.Num() <= i)
		{
			// 복사
			float _spawnPosition = i * 100.0f;
			TObjectPtr<AActor> _newRaderObject = GetWorld()->SpawnActor<AActor>(_raderObjectTemplate->GetClass(), FVector(_spawnPosition, _spawnPosition, _spawnPosition), FRotator::ZeroRotator);
			_newRaderObject->SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
			_raderObjectArray.Add(_newRaderObject);
			if (HasAuthority())
			{
				UE_LOG(LogTemp, Warning, TEXT("Spawn : %s"), *_newRaderObject->GetName());
			}
		}

		_raderObjectArray[i]->SetActorLocation(_raderCenter->GetComponentLocation() + _spaceObjectToRaderLocation);
		if (HasAuthority())
		{
			DrawDebugLine(GetWorld(), _raderCenter->GetComponentLocation(), _raderObjectArray[i]->GetActorLocation(), FColor::Red, false, _updateInterval);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		_updateTimerHandle,
		FTimerDelegate::CreateUObject(this, &ASpaceRader::UpdateSpaceObject),
		_updateInterval,
		false
	);
}