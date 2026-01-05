// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/DriveSeatRader.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"

// Sets default values
ADriveSeatRader::ADriveSeatRader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	_rootComponent->SetupAttachment(RootComponent);

	_spaceShipCenter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RaderCenter"));
	_spaceShipCenter->SetupAttachment(_rootComponent);
}

// Called when the game starts or when spawned
void ADriveSeatRader::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeDriveSeatRader();
}

// Called every frame
void ADriveSeatRader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADriveSeatRader::InitializeDriveSeatRader()
{
	AJHSGameMode* OutGameMode = nullptr;
	if (!UStaticFunctionLibrary::GetGameMode(OutGameMode))
		return;

	_spaceObjectManager = OutGameMode->GetSpaceObjectManager();
}

void ADriveSeatRader::UpdateDriveSeatRader()
{
	if (!_spaceObjectManager)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceRader: SpaceObjectManager is nullptr"));
		return;
	}
}