// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/DriveSeatRader.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

// Sets default values
ADriveSeatRader::ADriveSeatRader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_spaceShipMeshCenter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpaceShipMeshCenter"));
	_spaceShipMeshCenter->SetupAttachment(_rootComponent);
}

// Called when the game starts or when spawned
void ADriveSeatRader::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADriveSeatRader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// _spaceShip의 회전각의 부호 반대값으로 _driveSeatRaderCenter 회전
	if (_spaceShip != nullptr)
	{
		// _spaceShipCenter를 회전
		FRotator _spaceShipRotator = _spaceShip->GetActorRotation();
		_spaceShipMeshCenter->SetRelativeRotation(_spaceShipRotator);

		// _driveSeatRaderCenter를 회전
		//FVector _spaceShipRotation = FVector(_spaceShipRotator.Roll, _spaceShipRotator.Pitch, _spaceShipRotator.Yaw);

		//FVector _driveRaderRotation;
		//_driveRaderRotation.X = 360.f - FMath::Fmod((_spaceShipRotation.X + 360.f), 360.f);
		//_driveRaderRotation.Y = 360.f - FMath::Fmod((_spaceShipRotation.Y + 360.f), 360.f);
		//_driveRaderRotation.Z = 360.f - FMath::Fmod((_spaceShipRotation.Z + 360.f), 360.f);

		////언리얼 에디터에 디스플레이 로그
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString::Printf(TEXT("X: %f, Y: %f, Z: %f"), _driveRaderRotation.X, _driveRaderRotation.Y, _driveRaderRotation.Z));
		//FRotator _driveRaderRotator = FRotator::MakeFromEuler(_driveRaderRotation);
		//_driveSeatRaderCenter->SetRelativeRotation(_driveRaderRotator);
	}
}

void ADriveSeatRader::InitializeRader()
{
	USpaceManager* _outSpaceManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetSpaceManager(_outSpaceManager))
		return;

	if (_spaceShip == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ADriveSeatRader: SpaceShip is nullptr"));
		return;
	}

	_outSpaceManager->InitializeDriveRader(this, _spaceShip);
}

FString ADriveSeatRader::GetFilePathName()
{
	return ConstantLibrary::Resource.SpaceObject.DRIVE_RADER_FOLDER;
}

FString ADriveSeatRader::GetFileHeaderName()
{
	return ConstantLibrary::Resource.SpaceObject.DRIVE_RADER_HEADER;
}

void ADriveSeatRader::InitializeDriveRader(float DriveRaderRadius)
{
	FRaderData _newraderData;
	_newraderData.StandardActor = _spaceShip;
	_newraderData.RaderRenderRadius = DriveRaderRadius;
	StartRenderRader(_newraderData);
}